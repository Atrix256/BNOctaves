#pragma once

#include <d3d12.h>
#include <array>
#include <vector>
#include <unordered_map>
#include "DX12Utils/dxutils.h"

namespace BNOctaves
{
    using uint = unsigned int;
    using uint2 = std::array<uint, 2>;
    using uint3 = std::array<uint, 3>;
    using uint4 = std::array<uint, 4>;

    using int2 = std::array<int, 2>;
    using int3 = std::array<int, 3>;
    using int4 = std::array<int, 4>;
    using float2 = std::array<float, 2>;
    using float3 = std::array<float, 3>;
    using float4 = std::array<float, 4>;
    using float4x4 = std::array<std::array<float, 4>, 4>;

    enum class NoiseTypes: int
    {
        Blue,
        White,
        Binomial3x3,
        Box3x3,
        Box5x5,
        Perlin,
        R2,
        IGN,
        BlueReverse,
    };

    inline const char* EnumToString(NoiseTypes value, bool displayString = false)
    {
        switch(value)
        {
            case NoiseTypes::Blue: return displayString ? "Blue" : "Blue";
            case NoiseTypes::White: return displayString ? "White" : "White";
            case NoiseTypes::Binomial3x3: return displayString ? "Binomial3x3" : "Binomial3x3";
            case NoiseTypes::Box3x3: return displayString ? "Box3x3" : "Box3x3";
            case NoiseTypes::Box5x5: return displayString ? "Box5x5" : "Box5x5";
            case NoiseTypes::Perlin: return displayString ? "Perlin" : "Perlin";
            case NoiseTypes::R2: return displayString ? "R2" : "R2";
            case NoiseTypes::IGN: return displayString ? "IGN" : "IGN";
            case NoiseTypes::BlueReverse: return displayString ? "BlueReverse" : "BlueReverse";
            default: return nullptr;
        }
    }

    struct ContextInternal
    {
        ID3D12QueryHeap* m_TimestampQueryHeap = nullptr;
        ID3D12Resource* m_TimestampReadbackBuffer = nullptr;

        static ID3D12CommandSignature* s_commandSignatureDispatch;

        struct Struct__DisplayCSCB
        {
            uint BlueReverseStartSize = 64;
            unsigned int DifferentNoisePerOctave = false;  // If false, the same noise will be used for each octave. If true, a different noise, of the same type, will be used for each octave.
            int NoiseType = (int)NoiseTypes::Blue;  // The type of noise to use
            uint NumberOfOctaves = 3;  // How many octaves to use
            uint PerlinCellSize = 128;
            float2 PerlinMinMax = {0.0f, 1.0f};  // Perlin noise can go below zero which causes problems in this demo. To help that, this is the range of values which are mapped to [0,1]. Anything lower than 0 is clipped to 0 after the remapping.
            uint RNGSeed = 1337;  // A PRNG is used for various things, change this value to change thats eed.
        };

        struct Struct__Histogram_MakeCountsCSCB
        {
            uint Histogram_NumBuckets = 256;
            unsigned int Histogram_ZeroMinMaxBucket = false;  // If values are clamped to a min and max value, the min and max bucket will have too many counts in them. This option zeros them out to make the rest of the data easier to see.
            float2 _padding0 = {};  // Padding
        };

        struct Struct__Histogram_MakeGraphCSCB
        {
            uint2 Histogram_GraphSize = {256, 128};
            uint Histogram_NumBuckets = 256;
            float _padding0 = 0.000000f;  // Padding
        };

        struct Struct__Histogram_MakeMinMaxValueCSCB
        {
            unsigned int Histogram_AutoXAxisRange = true;
            float2 Histogram_XAxisRange = {0.0f, 1.0f};
            float _padding0 = 0.000000f;  // Padding
        };

        struct Struct__DFT_DFTCSCB
        {
            unsigned int DFT_RemoveDC = false;  // DC (0hz) is often a large spike that makes it hard to see the rest of the frequencies. Use this to set DC to zero.
            float3 _padding0 = {};  // Padding
        };

        struct Struct__DFT_NormalizeCSCB
        {
            unsigned int DFT_LogSpaceMagnitude = true;  // If true, show magnitude in log space
            float3 _padding0 = {};  // Padding
        };

        // Variables
        const float4 variable_ChannelDotProduct = {1.0f, 0.0f, 0.0f, 0.0f};  // Histogram and DFT need a scalar. Dotting a pixel value against this is how it gets that scalar

        static ID3D12PipelineState* computeShader_Histogram_InitMinMaxValue_pso;
        static ID3D12RootSignature* computeShader_Histogram_InitMinMaxValue_rootSig;

        static ID3D12PipelineState* computeShader_Histogram_InitCounts_pso;
        static ID3D12RootSignature* computeShader_Histogram_InitCounts_rootSig;

        ID3D12Resource* buffer_DFT_MaxMagnitude = nullptr;
        DXGI_FORMAT buffer_DFT_MaxMagnitude_format = DXGI_FORMAT_UNKNOWN; // For typed buffers, the type of the buffer
        unsigned int buffer_DFT_MaxMagnitude_stride = 0; // For structured buffers, the size of the structure
        unsigned int buffer_DFT_MaxMagnitude_count = 0; // How many items there are
        const D3D12_RESOURCE_STATES c_buffer_DFT_MaxMagnitude_endingState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

        static const D3D12_RESOURCE_FLAGS c_buffer_DFT_MaxMagnitude_flags =  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS; // Flags the buffer needs to have been created with

        ID3D12Resource* texture__loadedTexture_0 = nullptr;
        unsigned int texture__loadedTexture_0_size[3] = { 0, 0, 0 };
        unsigned int texture__loadedTexture_0_numMips = 0;
        DXGI_FORMAT texture__loadedTexture_0_format = DXGI_FORMAT_UNKNOWN;
        static const D3D12_RESOURCE_FLAGS texture__loadedTexture_0_flags =  D3D12_RESOURCE_FLAG_NONE;
        const D3D12_RESOURCE_STATES c_texture__loadedTexture_0_endingState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

        ID3D12Resource* texture__loadedTexture_1 = nullptr;
        unsigned int texture__loadedTexture_1_size[3] = { 0, 0, 0 };
        unsigned int texture__loadedTexture_1_numMips = 0;
        DXGI_FORMAT texture__loadedTexture_1_format = DXGI_FORMAT_UNKNOWN;
        static const D3D12_RESOURCE_FLAGS texture__loadedTexture_1_flags =  D3D12_RESOURCE_FLAG_NONE;
        const D3D12_RESOURCE_STATES c_texture__loadedTexture_1_endingState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

        ID3D12Resource* texture__loadedTexture_2 = nullptr;
        unsigned int texture__loadedTexture_2_size[3] = { 0, 0, 0 };
        unsigned int texture__loadedTexture_2_numMips = 0;
        DXGI_FORMAT texture__loadedTexture_2_format = DXGI_FORMAT_UNKNOWN;
        static const D3D12_RESOURCE_FLAGS texture__loadedTexture_2_flags =  D3D12_RESOURCE_FLAG_NONE;
        const D3D12_RESOURCE_STATES c_texture__loadedTexture_2_endingState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

        ID3D12Resource* texture__loadedTexture_3 = nullptr;
        unsigned int texture__loadedTexture_3_size[3] = { 0, 0, 0 };
        unsigned int texture__loadedTexture_3_numMips = 0;
        DXGI_FORMAT texture__loadedTexture_3_format = DXGI_FORMAT_UNKNOWN;
        static const D3D12_RESOURCE_FLAGS texture__loadedTexture_3_flags =  D3D12_RESOURCE_FLAG_NONE;
        const D3D12_RESOURCE_STATES c_texture__loadedTexture_3_endingState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

        Struct__DisplayCSCB constantBuffer__DisplayCSCB_cpu;
        ID3D12Resource* constantBuffer__DisplayCSCB = nullptr;

        static ID3D12PipelineState* computeShader_Display_pso;
        static ID3D12RootSignature* computeShader_Display_rootSig;

        Struct__Histogram_MakeCountsCSCB constantBuffer__Histogram_MakeCountsCSCB_cpu;
        ID3D12Resource* constantBuffer__Histogram_MakeCountsCSCB = nullptr;

        Struct__Histogram_MakeGraphCSCB constantBuffer__Histogram_MakeGraphCSCB_cpu;
        ID3D12Resource* constantBuffer__Histogram_MakeGraphCSCB = nullptr;

        Struct__Histogram_MakeMinMaxValueCSCB constantBuffer__Histogram_MakeMinMaxValueCSCB_cpu;
        ID3D12Resource* constantBuffer__Histogram_MakeMinMaxValueCSCB = nullptr;

        static ID3D12PipelineState* computeShader_Histogram_MakeMinMaxValue_pso;
        static ID3D12RootSignature* computeShader_Histogram_MakeMinMaxValue_rootSig;

        static ID3D12PipelineState* computeShader_Histogram_MakeCounts_pso;
        static ID3D12RootSignature* computeShader_Histogram_MakeCounts_rootSig;

        static ID3D12PipelineState* computeShader_Histogram_MakeGraph_pso;
        static ID3D12RootSignature* computeShader_Histogram_MakeGraph_rootSig;

        Struct__DFT_DFTCSCB constantBuffer__DFT_DFTCSCB_cpu;
        ID3D12Resource* constantBuffer__DFT_DFTCSCB = nullptr;

        static ID3D12PipelineState* computeShader_DFT_DFT_pso;
        static ID3D12RootSignature* computeShader_DFT_DFT_rootSig;

        Struct__DFT_NormalizeCSCB constantBuffer__DFT_NormalizeCSCB_cpu;
        ID3D12Resource* constantBuffer__DFT_NormalizeCSCB = nullptr;

        static ID3D12PipelineState* computeShader_DFT_Normalize_pso;
        static ID3D12RootSignature* computeShader_DFT_Normalize_rootSig;

        static ID3D12PipelineState* computeShader_MakeCombinedOutput_pso;
        static ID3D12RootSignature* computeShader_MakeCombinedOutput_rootSig;

        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_RTVCache;
        std::unordered_map<DX12Utils::SubResourceHeapAllocationInfo, int, DX12Utils::SubResourceHeapAllocationInfo> m_DSVCache;

        // Freed on destruction of the context
        std::vector<ID3D12Resource*> m_managedResources;
    };
};
