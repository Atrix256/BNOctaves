#include "../public/technique.h"
#include "DX12Utils/dxutils.h"
#include "DX12Utils/DelayedReleaseTracker.h"
#include "DX12Utils/HeapAllocationTracker.h"
#include "DX12Utils/TextureCache.h"

#include <vector>
#include <chrono>

namespace BNOctaves
{
    static std::vector<Context*> s_allContexts;

    static DX12Utils::Heap                  s_srvHeap;
    static DX12Utils::Heap                  s_rtvHeap;
    static DX12Utils::Heap                  s_dsvHeap;
    static DX12Utils::UploadBufferTracker   s_ubTracker;
    static DX12Utils::DelayedReleaseTracker s_delayedRelease;
    static DX12Utils::HeapAllocationTracker s_heapAllocationTrackerRTV;
    static DX12Utils::HeapAllocationTracker s_heapAllocationTrackerDSV;

    TLogFn Context::LogFn = [] (LogLevel level, const char* msg, ...) {};
    TPerfEventBeginFn Context::PerfEventBeginFn = [] (const char* name, ID3D12GraphicsCommandList* commandList, int index) {};
    TPerfEventEndFn Context::PerfEventEndFn = [] (ID3D12GraphicsCommandList* commandList) {};

    std::wstring Context::s_techniqueLocation = L"./";
    static unsigned int s_timerIndex = 0;

    ID3D12CommandSignature* ContextInternal::s_commandSignatureDispatch = nullptr;

    ID3D12PipelineState* ContextInternal::computeShader_Histogram_InitMinMaxValue_pso = nullptr;
    ID3D12RootSignature* ContextInternal::computeShader_Histogram_InitMinMaxValue_rootSig = nullptr;

    ID3D12PipelineState* ContextInternal::computeShader_Histogram_InitCounts_pso = nullptr;
    ID3D12RootSignature* ContextInternal::computeShader_Histogram_InitCounts_rootSig = nullptr;

    ID3D12PipelineState* ContextInternal::computeShader_Display_pso = nullptr;
    ID3D12RootSignature* ContextInternal::computeShader_Display_rootSig = nullptr;

    ID3D12PipelineState* ContextInternal::computeShader_Histogram_MakeMinMaxValue_pso = nullptr;
    ID3D12RootSignature* ContextInternal::computeShader_Histogram_MakeMinMaxValue_rootSig = nullptr;

    ID3D12PipelineState* ContextInternal::computeShader_Histogram_MakeCounts_pso = nullptr;
    ID3D12RootSignature* ContextInternal::computeShader_Histogram_MakeCounts_rootSig = nullptr;

    ID3D12PipelineState* ContextInternal::computeShader_Histogram_MakeGraph_pso = nullptr;
    ID3D12RootSignature* ContextInternal::computeShader_Histogram_MakeGraph_rootSig = nullptr;

    ID3D12PipelineState* ContextInternal::computeShader_DFT_DFT_pso = nullptr;
    ID3D12RootSignature* ContextInternal::computeShader_DFT_DFT_rootSig = nullptr;

    ID3D12PipelineState* ContextInternal::computeShader_DFT_Normalize_pso = nullptr;
    ID3D12RootSignature* ContextInternal::computeShader_DFT_Normalize_rootSig = nullptr;

    ID3D12PipelineState* ContextInternal::computeShader_MakeCombinedOutput_pso = nullptr;
    ID3D12RootSignature* ContextInternal::computeShader_MakeCombinedOutput_rootSig = nullptr;

    template <typename T>
    T Pow2GE(const T& A)
    {
        float f = std::log2(float(A));
        f = std::ceilf(f);
        return (T)std::pow(2.0f, f);
    }

    bool CreateShared(ID3D12Device* device)
    {

        // Compute Shader: Histogram_InitMinMaxValue
        {
            D3D12_STATIC_SAMPLER_DESC* samplers = nullptr;

            D3D12_DESCRIPTOR_RANGE ranges[1];

            // MinMaxValue
            ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            ranges[0].NumDescriptors = 1;
            ranges[0].BaseShaderRegister = 0;
            ranges[0].RegisterSpace = 0;
            ranges[0].OffsetInDescriptorsFromTableStart = 0;

            if(!DX12Utils::MakeRootSig(device, ranges, 1, samplers, 0, &ContextInternal::computeShader_Histogram_InitMinMaxValue_rootSig, (c_debugNames ? L"Histogram_InitMinMaxValue" : nullptr), Context::LogFn))
                return false;

            D3D_SHADER_MACRO defines[] = {
                { "__GigiDispatchMultiply", "uint3(1,1,1)" },
                { "__GigiDispatchDivide", "uint3(1,1,1)" },
                { "__GigiDispatchPreAdd", "uint3(0,0,0)" },
                { "__GigiDispatchPostAdd", "uint3(0,0,0)" },
                { nullptr, nullptr }
            };

            if(!DX12Utils::MakeComputePSO_DXC(device, Context::s_techniqueLocation.c_str(), L"shaders/Histogram/InitMinMaxValueCS.hlsl", "csmain", "cs_6_1", defines,
               ContextInternal::computeShader_Histogram_InitMinMaxValue_rootSig, &ContextInternal::computeShader_Histogram_InitMinMaxValue_pso, c_debugShaders, (c_debugNames ? L"Histogram_InitMinMaxValue" : nullptr), Context::LogFn))
                return false;
        }

        // Compute Shader: Histogram_InitCounts
        {
            D3D12_STATIC_SAMPLER_DESC* samplers = nullptr;

            D3D12_DESCRIPTOR_RANGE ranges[1];

            // Counts
            ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            ranges[0].NumDescriptors = 1;
            ranges[0].BaseShaderRegister = 0;
            ranges[0].RegisterSpace = 0;
            ranges[0].OffsetInDescriptorsFromTableStart = 0;

            if(!DX12Utils::MakeRootSig(device, ranges, 1, samplers, 0, &ContextInternal::computeShader_Histogram_InitCounts_rootSig, (c_debugNames ? L"Histogram_InitCounts" : nullptr), Context::LogFn))
                return false;

            D3D_SHADER_MACRO defines[] = {
                { "__GigiDispatchMultiply", "uint3(1,1,1)" },
                { "__GigiDispatchDivide", "uint3(1,1,1)" },
                { "__GigiDispatchPreAdd", "uint3(0,0,0)" },
                { "__GigiDispatchPostAdd", "uint3(0,0,0)" },
                { nullptr, nullptr }
            };

            if(!DX12Utils::MakeComputePSO_DXC(device, Context::s_techniqueLocation.c_str(), L"shaders/Histogram/InitCountsCS.hlsl", "csmain", "cs_6_1", defines,
               ContextInternal::computeShader_Histogram_InitCounts_rootSig, &ContextInternal::computeShader_Histogram_InitCounts_pso, c_debugShaders, (c_debugNames ? L"Histogram_InitCounts" : nullptr), Context::LogFn))
                return false;
        }

        // Compute Shader: Display
        {
            D3D12_STATIC_SAMPLER_DESC* samplers = nullptr;

            D3D12_DESCRIPTOR_RANGE ranges[7];

            // Output
            ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            ranges[0].NumDescriptors = 1;
            ranges[0].BaseShaderRegister = 0;
            ranges[0].RegisterSpace = 0;
            ranges[0].OffsetInDescriptorsFromTableStart = 0;

            // OutputF
            ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            ranges[1].NumDescriptors = 1;
            ranges[1].BaseShaderRegister = 1;
            ranges[1].RegisterSpace = 0;
            ranges[1].OffsetInDescriptorsFromTableStart = 1;

            // _loadedTexture_0
            ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            ranges[2].NumDescriptors = 1;
            ranges[2].BaseShaderRegister = 0;
            ranges[2].RegisterSpace = 0;
            ranges[2].OffsetInDescriptorsFromTableStart = 2;

            // _loadedTexture_1
            ranges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            ranges[3].NumDescriptors = 1;
            ranges[3].BaseShaderRegister = 1;
            ranges[3].RegisterSpace = 0;
            ranges[3].OffsetInDescriptorsFromTableStart = 3;

            // _loadedTexture_2
            ranges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            ranges[4].NumDescriptors = 1;
            ranges[4].BaseShaderRegister = 2;
            ranges[4].RegisterSpace = 0;
            ranges[4].OffsetInDescriptorsFromTableStart = 4;

            // _loadedTexture_3
            ranges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            ranges[5].NumDescriptors = 1;
            ranges[5].BaseShaderRegister = 3;
            ranges[5].RegisterSpace = 0;
            ranges[5].OffsetInDescriptorsFromTableStart = 5;

            // _DisplayCSCB
            ranges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            ranges[6].NumDescriptors = 1;
            ranges[6].BaseShaderRegister = 0;
            ranges[6].RegisterSpace = 0;
            ranges[6].OffsetInDescriptorsFromTableStart = 6;

            if(!DX12Utils::MakeRootSig(device, ranges, 7, samplers, 0, &ContextInternal::computeShader_Display_rootSig, (c_debugNames ? L"Display" : nullptr), Context::LogFn))
                return false;

            D3D_SHADER_MACRO defines[] = {
                { "__GigiDispatchMultiply", "uint3(1,1,1)" },
                { "__GigiDispatchDivide", "uint3(1,1,1)" },
                { "__GigiDispatchPreAdd", "uint3(0,0,0)" },
                { "__GigiDispatchPostAdd", "uint3(0,0,0)" },
                { nullptr, nullptr }
            };

            if(!DX12Utils::MakeComputePSO_DXC(device, Context::s_techniqueLocation.c_str(), L"shaders/DisplayCS.hlsl", "csmain", "cs_6_1", defines,
               ContextInternal::computeShader_Display_rootSig, &ContextInternal::computeShader_Display_pso, c_debugShaders, (c_debugNames ? L"Display" : nullptr), Context::LogFn))
                return false;
        }

        // Compute Shader: Histogram_MakeMinMaxValue
        {
            D3D12_STATIC_SAMPLER_DESC* samplers = nullptr;

            D3D12_DESCRIPTOR_RANGE ranges[3];

            // Input
            ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            ranges[0].NumDescriptors = 1;
            ranges[0].BaseShaderRegister = 0;
            ranges[0].RegisterSpace = 0;
            ranges[0].OffsetInDescriptorsFromTableStart = 0;

            // MinMaxValue
            ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            ranges[1].NumDescriptors = 1;
            ranges[1].BaseShaderRegister = 0;
            ranges[1].RegisterSpace = 0;
            ranges[1].OffsetInDescriptorsFromTableStart = 1;

            // _Histogram_MakeMinMaxValueCSCB
            ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            ranges[2].NumDescriptors = 1;
            ranges[2].BaseShaderRegister = 0;
            ranges[2].RegisterSpace = 0;
            ranges[2].OffsetInDescriptorsFromTableStart = 2;

            if(!DX12Utils::MakeRootSig(device, ranges, 3, samplers, 0, &ContextInternal::computeShader_Histogram_MakeMinMaxValue_rootSig, (c_debugNames ? L"Histogram_MakeMinMaxValue" : nullptr), Context::LogFn))
                return false;

            D3D_SHADER_MACRO defines[] = {
                { "__GigiDispatchMultiply", "uint3(1,1,1)" },
                { "__GigiDispatchDivide", "uint3(1,1,1)" },
                { "__GigiDispatchPreAdd", "uint3(0,0,0)" },
                { "__GigiDispatchPostAdd", "uint3(0,0,0)" },
                { nullptr, nullptr }
            };

            if(!DX12Utils::MakeComputePSO_DXC(device, Context::s_techniqueLocation.c_str(), L"shaders/Histogram/MakeMinMaxValueCS.hlsl", "csmain", "cs_6_1", defines,
               ContextInternal::computeShader_Histogram_MakeMinMaxValue_rootSig, &ContextInternal::computeShader_Histogram_MakeMinMaxValue_pso, c_debugShaders, (c_debugNames ? L"Histogram_MakeMinMaxValue" : nullptr), Context::LogFn))
                return false;
        }

        // Compute Shader: Histogram_MakeCounts
        {
            D3D12_STATIC_SAMPLER_DESC* samplers = nullptr;

            D3D12_DESCRIPTOR_RANGE ranges[4];

            // Input
            ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            ranges[0].NumDescriptors = 1;
            ranges[0].BaseShaderRegister = 0;
            ranges[0].RegisterSpace = 0;
            ranges[0].OffsetInDescriptorsFromTableStart = 0;

            // MinMaxValue
            ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            ranges[1].NumDescriptors = 1;
            ranges[1].BaseShaderRegister = 1;
            ranges[1].RegisterSpace = 0;
            ranges[1].OffsetInDescriptorsFromTableStart = 1;

            // Counts
            ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            ranges[2].NumDescriptors = 1;
            ranges[2].BaseShaderRegister = 0;
            ranges[2].RegisterSpace = 0;
            ranges[2].OffsetInDescriptorsFromTableStart = 2;

            // _Histogram_MakeCountsCSCB
            ranges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            ranges[3].NumDescriptors = 1;
            ranges[3].BaseShaderRegister = 0;
            ranges[3].RegisterSpace = 0;
            ranges[3].OffsetInDescriptorsFromTableStart = 3;

            if(!DX12Utils::MakeRootSig(device, ranges, 4, samplers, 0, &ContextInternal::computeShader_Histogram_MakeCounts_rootSig, (c_debugNames ? L"Histogram_MakeCounts" : nullptr), Context::LogFn))
                return false;

            D3D_SHADER_MACRO defines[] = {
                { "__GigiDispatchMultiply", "uint3(1,1,1)" },
                { "__GigiDispatchDivide", "uint3(1,1,1)" },
                { "__GigiDispatchPreAdd", "uint3(0,0,0)" },
                { "__GigiDispatchPostAdd", "uint3(0,0,0)" },
                { nullptr, nullptr }
            };

            if(!DX12Utils::MakeComputePSO_DXC(device, Context::s_techniqueLocation.c_str(), L"shaders/Histogram/MakeCountsCS.hlsl", "csmain", "cs_6_1", defines,
               ContextInternal::computeShader_Histogram_MakeCounts_rootSig, &ContextInternal::computeShader_Histogram_MakeCounts_pso, c_debugShaders, (c_debugNames ? L"Histogram_MakeCounts" : nullptr), Context::LogFn))
                return false;
        }

        // Compute Shader: Histogram_MakeGraph
        {
            D3D12_STATIC_SAMPLER_DESC* samplers = nullptr;

            D3D12_DESCRIPTOR_RANGE ranges[3];

            // Counts
            ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            ranges[0].NumDescriptors = 1;
            ranges[0].BaseShaderRegister = 0;
            ranges[0].RegisterSpace = 0;
            ranges[0].OffsetInDescriptorsFromTableStart = 0;

            // Output
            ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            ranges[1].NumDescriptors = 1;
            ranges[1].BaseShaderRegister = 0;
            ranges[1].RegisterSpace = 0;
            ranges[1].OffsetInDescriptorsFromTableStart = 1;

            // _Histogram_MakeGraphCSCB
            ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            ranges[2].NumDescriptors = 1;
            ranges[2].BaseShaderRegister = 0;
            ranges[2].RegisterSpace = 0;
            ranges[2].OffsetInDescriptorsFromTableStart = 2;

            if(!DX12Utils::MakeRootSig(device, ranges, 3, samplers, 0, &ContextInternal::computeShader_Histogram_MakeGraph_rootSig, (c_debugNames ? L"Histogram_MakeGraph" : nullptr), Context::LogFn))
                return false;

            D3D_SHADER_MACRO defines[] = {
                { "__GigiDispatchMultiply", "uint3(1,1,1)" },
                { "__GigiDispatchDivide", "uint3(1,1,1)" },
                { "__GigiDispatchPreAdd", "uint3(0,0,0)" },
                { "__GigiDispatchPostAdd", "uint3(0,0,0)" },
                { nullptr, nullptr }
            };

            if(!DX12Utils::MakeComputePSO_DXC(device, Context::s_techniqueLocation.c_str(), L"shaders/Histogram/MakeGraphCS.hlsl", "csmain", "cs_6_1", defines,
               ContextInternal::computeShader_Histogram_MakeGraph_rootSig, &ContextInternal::computeShader_Histogram_MakeGraph_pso, c_debugShaders, (c_debugNames ? L"Histogram_MakeGraph" : nullptr), Context::LogFn))
                return false;
        }

        // Compute Shader: DFT_DFT
        {
            D3D12_STATIC_SAMPLER_DESC* samplers = nullptr;

            D3D12_DESCRIPTOR_RANGE ranges[4];

            // Input
            ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            ranges[0].NumDescriptors = 1;
            ranges[0].BaseShaderRegister = 0;
            ranges[0].RegisterSpace = 0;
            ranges[0].OffsetInDescriptorsFromTableStart = 0;

            // Output
            ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            ranges[1].NumDescriptors = 1;
            ranges[1].BaseShaderRegister = 0;
            ranges[1].RegisterSpace = 0;
            ranges[1].OffsetInDescriptorsFromTableStart = 1;

            // MaxMagnitude
            ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            ranges[2].NumDescriptors = 1;
            ranges[2].BaseShaderRegister = 1;
            ranges[2].RegisterSpace = 0;
            ranges[2].OffsetInDescriptorsFromTableStart = 2;

            // _DFT_DFTCSCB
            ranges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            ranges[3].NumDescriptors = 1;
            ranges[3].BaseShaderRegister = 0;
            ranges[3].RegisterSpace = 0;
            ranges[3].OffsetInDescriptorsFromTableStart = 3;

            if(!DX12Utils::MakeRootSig(device, ranges, 4, samplers, 0, &ContextInternal::computeShader_DFT_DFT_rootSig, (c_debugNames ? L"DFT_DFT" : nullptr), Context::LogFn))
                return false;

            D3D_SHADER_MACRO defines[] = {
                { "__GigiDispatchMultiply", "uint3(1,1,1)" },
                { "__GigiDispatchDivide", "uint3(1,1,1)" },
                { "__GigiDispatchPreAdd", "uint3(0,0,0)" },
                { "__GigiDispatchPostAdd", "uint3(0,0,0)" },
                { nullptr, nullptr }
            };

            if(!DX12Utils::MakeComputePSO_DXC(device, Context::s_techniqueLocation.c_str(), L"shaders/DFT/DFTCS.hlsl", "csmain", "cs_6_1", defines,
               ContextInternal::computeShader_DFT_DFT_rootSig, &ContextInternal::computeShader_DFT_DFT_pso, c_debugShaders, (c_debugNames ? L"DFT_DFT" : nullptr), Context::LogFn))
                return false;
        }

        // Compute Shader: DFT_Normalize
        {
            D3D12_STATIC_SAMPLER_DESC* samplers = nullptr;

            D3D12_DESCRIPTOR_RANGE ranges[3];

            // DFTMagnitude
            ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            ranges[0].NumDescriptors = 1;
            ranges[0].BaseShaderRegister = 0;
            ranges[0].RegisterSpace = 0;
            ranges[0].OffsetInDescriptorsFromTableStart = 0;

            // MaxMagnitude
            ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            ranges[1].NumDescriptors = 1;
            ranges[1].BaseShaderRegister = 0;
            ranges[1].RegisterSpace = 0;
            ranges[1].OffsetInDescriptorsFromTableStart = 1;

            // _DFT_NormalizeCSCB
            ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            ranges[2].NumDescriptors = 1;
            ranges[2].BaseShaderRegister = 0;
            ranges[2].RegisterSpace = 0;
            ranges[2].OffsetInDescriptorsFromTableStart = 2;

            if(!DX12Utils::MakeRootSig(device, ranges, 3, samplers, 0, &ContextInternal::computeShader_DFT_Normalize_rootSig, (c_debugNames ? L"DFT_Normalize" : nullptr), Context::LogFn))
                return false;

            D3D_SHADER_MACRO defines[] = {
                { "__GigiDispatchMultiply", "uint3(1,1,1)" },
                { "__GigiDispatchDivide", "uint3(1,1,1)" },
                { "__GigiDispatchPreAdd", "uint3(0,0,0)" },
                { "__GigiDispatchPostAdd", "uint3(0,0,0)" },
                { nullptr, nullptr }
            };

            if(!DX12Utils::MakeComputePSO_DXC(device, Context::s_techniqueLocation.c_str(), L"shaders/DFT/NormalizeCS.hlsl", "csmain", "cs_6_1", defines,
               ContextInternal::computeShader_DFT_Normalize_rootSig, &ContextInternal::computeShader_DFT_Normalize_pso, c_debugShaders, (c_debugNames ? L"DFT_Normalize" : nullptr), Context::LogFn))
                return false;
        }

        // Compute Shader: MakeCombinedOutput
        {
            D3D12_STATIC_SAMPLER_DESC* samplers = nullptr;

            D3D12_DESCRIPTOR_RANGE ranges[4];

            // CombinedOutput
            ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            ranges[0].NumDescriptors = 1;
            ranges[0].BaseShaderRegister = 0;
            ranges[0].RegisterSpace = 0;
            ranges[0].OffsetInDescriptorsFromTableStart = 0;

            // Noise
            ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            ranges[1].NumDescriptors = 1;
            ranges[1].BaseShaderRegister = 0;
            ranges[1].RegisterSpace = 0;
            ranges[1].OffsetInDescriptorsFromTableStart = 1;

            // Histogram
            ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            ranges[2].NumDescriptors = 1;
            ranges[2].BaseShaderRegister = 1;
            ranges[2].RegisterSpace = 0;
            ranges[2].OffsetInDescriptorsFromTableStart = 2;

            // DFT
            ranges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            ranges[3].NumDescriptors = 1;
            ranges[3].BaseShaderRegister = 2;
            ranges[3].RegisterSpace = 0;
            ranges[3].OffsetInDescriptorsFromTableStart = 3;

            if(!DX12Utils::MakeRootSig(device, ranges, 4, samplers, 0, &ContextInternal::computeShader_MakeCombinedOutput_rootSig, (c_debugNames ? L"MakeCombinedOutput" : nullptr), Context::LogFn))
                return false;

            D3D_SHADER_MACRO defines[] = {
                { "__GigiDispatchMultiply", "uint3(1,1,1)" },
                { "__GigiDispatchDivide", "uint3(1,1,1)" },
                { "__GigiDispatchPreAdd", "uint3(0,0,0)" },
                { "__GigiDispatchPostAdd", "uint3(0,0,0)" },
                { nullptr, nullptr }
            };

            if(!DX12Utils::MakeComputePSO_DXC(device, Context::s_techniqueLocation.c_str(), L"shaders/MakeCombinedOutputCS.hlsl", "csmain", "cs_6_1", defines,
               ContextInternal::computeShader_MakeCombinedOutput_rootSig, &ContextInternal::computeShader_MakeCombinedOutput_pso, c_debugShaders, (c_debugNames ? L"MakeCombinedOutput" : nullptr), Context::LogFn))
                return false;
        }

        // Create heaps
        if (c_numSRVDescriptors > 0 && !CreateHeap(s_srvHeap, device, c_numSRVDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, Context::LogFn))
            return false;

        if (c_numRTVDescriptors > 0 && !CreateHeap(s_rtvHeap, device, c_numRTVDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, Context::LogFn))
            return false;

        if (c_numDSVDescriptors > 0 && !CreateHeap(s_dsvHeap, device, c_numDSVDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, Context::LogFn))
            return false;

        s_heapAllocationTrackerRTV.Init(s_rtvHeap.m_heap, c_numRTVDescriptors, (int)device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
        s_heapAllocationTrackerDSV.Init(s_dsvHeap.m_heap, c_numDSVDescriptors, (int)device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV));

        // create indirect dispatch command
        {
            D3D12_INDIRECT_ARGUMENT_DESC dispatchArg = {};
            dispatchArg.Type						 = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;

            D3D12_COMMAND_SIGNATURE_DESC dispatchDesc = {};
            dispatchDesc.ByteStride					  = sizeof(uint32_t) * 3;
            dispatchDesc.NumArgumentDescs			  = 1;
            dispatchDesc.pArgumentDescs				  = &dispatchArg;
            dispatchDesc.NodeMask					  = 0x0;

            device->CreateCommandSignature(
                &dispatchDesc,
                nullptr,
                IID_PPV_ARGS(&ContextInternal::s_commandSignatureDispatch));
        }

        return true;
    }

    void DestroyShared()
    {

        if(ContextInternal::computeShader_Histogram_InitMinMaxValue_pso)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_Histogram_InitMinMaxValue_pso);
            ContextInternal::computeShader_Histogram_InitMinMaxValue_pso = nullptr;
        }

        if(ContextInternal::computeShader_Histogram_InitMinMaxValue_rootSig)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_Histogram_InitMinMaxValue_rootSig);
            ContextInternal::computeShader_Histogram_InitMinMaxValue_rootSig = nullptr;
        }

        if(ContextInternal::computeShader_Histogram_InitCounts_pso)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_Histogram_InitCounts_pso);
            ContextInternal::computeShader_Histogram_InitCounts_pso = nullptr;
        }

        if(ContextInternal::computeShader_Histogram_InitCounts_rootSig)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_Histogram_InitCounts_rootSig);
            ContextInternal::computeShader_Histogram_InitCounts_rootSig = nullptr;
        }

        if(ContextInternal::computeShader_Display_pso)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_Display_pso);
            ContextInternal::computeShader_Display_pso = nullptr;
        }

        if(ContextInternal::computeShader_Display_rootSig)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_Display_rootSig);
            ContextInternal::computeShader_Display_rootSig = nullptr;
        }

        if(ContextInternal::computeShader_Histogram_MakeMinMaxValue_pso)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_Histogram_MakeMinMaxValue_pso);
            ContextInternal::computeShader_Histogram_MakeMinMaxValue_pso = nullptr;
        }

        if(ContextInternal::computeShader_Histogram_MakeMinMaxValue_rootSig)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_Histogram_MakeMinMaxValue_rootSig);
            ContextInternal::computeShader_Histogram_MakeMinMaxValue_rootSig = nullptr;
        }

        if(ContextInternal::computeShader_Histogram_MakeCounts_pso)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_Histogram_MakeCounts_pso);
            ContextInternal::computeShader_Histogram_MakeCounts_pso = nullptr;
        }

        if(ContextInternal::computeShader_Histogram_MakeCounts_rootSig)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_Histogram_MakeCounts_rootSig);
            ContextInternal::computeShader_Histogram_MakeCounts_rootSig = nullptr;
        }

        if(ContextInternal::computeShader_Histogram_MakeGraph_pso)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_Histogram_MakeGraph_pso);
            ContextInternal::computeShader_Histogram_MakeGraph_pso = nullptr;
        }

        if(ContextInternal::computeShader_Histogram_MakeGraph_rootSig)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_Histogram_MakeGraph_rootSig);
            ContextInternal::computeShader_Histogram_MakeGraph_rootSig = nullptr;
        }

        if(ContextInternal::computeShader_DFT_DFT_pso)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_DFT_DFT_pso);
            ContextInternal::computeShader_DFT_DFT_pso = nullptr;
        }

        if(ContextInternal::computeShader_DFT_DFT_rootSig)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_DFT_DFT_rootSig);
            ContextInternal::computeShader_DFT_DFT_rootSig = nullptr;
        }

        if(ContextInternal::computeShader_DFT_Normalize_pso)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_DFT_Normalize_pso);
            ContextInternal::computeShader_DFT_Normalize_pso = nullptr;
        }

        if(ContextInternal::computeShader_DFT_Normalize_rootSig)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_DFT_Normalize_rootSig);
            ContextInternal::computeShader_DFT_Normalize_rootSig = nullptr;
        }

        if(ContextInternal::computeShader_MakeCombinedOutput_pso)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_MakeCombinedOutput_pso);
            ContextInternal::computeShader_MakeCombinedOutput_pso = nullptr;
        }

        if(ContextInternal::computeShader_MakeCombinedOutput_rootSig)
        {
            s_delayedRelease.Add(ContextInternal::computeShader_MakeCombinedOutput_rootSig);
            ContextInternal::computeShader_MakeCombinedOutput_rootSig = nullptr;
        }

        // Clear out heap trackers
        s_heapAllocationTrackerRTV.Release();
        s_heapAllocationTrackerDSV.Release();

        // Destroy Heaps
        DestroyHeap(s_srvHeap);
        DestroyHeap(s_rtvHeap);
        DestroyHeap(s_dsvHeap);

        // Destroy any upload buffers
        s_ubTracker.Release();

        // Finish any delayed release
        s_delayedRelease.Release();

        // Destroy indirect dispatch command
        if (ContextInternal::s_commandSignatureDispatch)
        {
            ContextInternal::s_commandSignatureDispatch->Release();
            ContextInternal::s_commandSignatureDispatch = nullptr;
        }
    }

    Context* CreateContext(ID3D12Device* device)
    {
        if (s_allContexts.size() == 0)
        {
            if (!CreateShared(device))
                return nullptr;
        }

        Context* ret = new Context;
        s_allContexts.push_back(ret);
        return ret;
    }

    void DestroyContext(Context* context)
    {
        s_allContexts.erase(std::remove(s_allContexts.begin(), s_allContexts.end(), context), s_allContexts.end());
        delete context;
        if (s_allContexts.size() == 0)
            DestroyShared();
    }

    void OnNewFrame(int framesInFlight)
    {
        s_delayedRelease.OnNewFrame(framesInFlight);
        s_ubTracker.OnNewFrame(framesInFlight);
        s_heapAllocationTrackerRTV.OnNewFrame(framesInFlight);
        s_heapAllocationTrackerDSV.OnNewFrame(framesInFlight);
    }

    int Context::GetContextCount()
    {
        return (int)s_allContexts.size();
    }

    Context* Context::GetContext(int index)
    {
        if (index >= 0 && index < GetContextCount())
            return s_allContexts[index];
        else
            return nullptr;
    }

    ID3D12Resource* Context::CreateManagedBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, const void* data, size_t size, const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState)
    {
        // Make a buffer and have the context manage it
        ID3D12Resource* ret = DX12Utils::CreateBuffer(
            device,
            (unsigned int)size,
            flags,
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_HEAP_TYPE_DEFAULT,
            debugName,
            Context::LogFn
        );
        AddManagedResource(ret);

        // Copy the data to the buffer if we should
        if (data != nullptr && size > 0)
            UploadBufferData(device, commandList, ret, D3D12_RESOURCE_STATE_COPY_DEST, data, (unsigned int)size);

        // Do a resource transition if we should
        if (desiredState != D3D12_RESOURCE_STATE_COPY_DEST)
        {
            D3D12_RESOURCE_BARRIER barrier;

            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = ret;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.StateAfter = desiredState;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, &barrier);
        }

        // return the resource
        return ret;
    }

    ID3D12Resource* Context::CreateManagedTexture(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, DXGI_FORMAT format, const unsigned int size[3], unsigned int numMips, DX12Utils::ResourceType resourceType, const void* initialData, const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState)
    {
        // Create a texture
        ID3D12Resource* ret = DX12Utils::CreateTexture(device, size, numMips, format, flags, D3D12_RESOURCE_STATE_COPY_DEST, resourceType, debugName, Context::LogFn);
        AddManagedResource(ret);

        // copy initial data in, if we should
        if (initialData != nullptr)
        {
            DX12Utils::DXGI_FORMAT_Info formatInfo = DX12Utils::Get_DXGI_FORMAT_Info(format, Context::LogFn);
            UploadTextureData(device, commandList, ret, D3D12_RESOURCE_STATE_COPY_DEST, initialData, size[0] * formatInfo.bytesPerPixel);
        }

        // Put the resource into the desired state
        if (desiredState != D3D12_RESOURCE_STATE_COPY_DEST)
        {
            D3D12_RESOURCE_BARRIER barrier;

            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = ret;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.StateAfter = desiredState;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, &barrier);
        }

        return ret;
    }

    ID3D12Resource* Context::CreateManagedTextureAndClear(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, DXGI_FORMAT format, const unsigned int size[3], unsigned int numMips, DX12Utils::ResourceType resourceType, void* clearValue, size_t clearValueSize, const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState)
    {
        // Make sure the clear value is the correct size
        DX12Utils::DXGI_FORMAT_Info formatInfo = DX12Utils::Get_DXGI_FORMAT_Info(format, Context::LogFn);
        if (clearValue != nullptr && clearValueSize > 0 && clearValueSize != formatInfo.bytesPerPixel)
            return nullptr;

        // Copy data into the resource
        std::vector<unsigned char> expandedClearValue;
        void* initialData = nullptr;
        if (clearValue != nullptr && clearValueSize > 0)
        {
            expandedClearValue.resize(size[0] * size[1] * size[2] * formatInfo.bytesPerPixel);
            unsigned char* dest = expandedClearValue.data();
            for (size_t i = 0; i < size[0] * size[1] * size[2]; ++i)
            {
                memcpy(dest, clearValue, formatInfo.bytesPerPixel);
                dest += formatInfo.bytesPerPixel;
            }
            initialData = expandedClearValue.data();
        }

        // make and return the texture
        return CreateManagedTexture(device, commandList, flags, format, size, numMips, resourceType, initialData, debugName, desiredState);
    }

    ID3D12Resource* Context::CreateManagedTextureFromFile(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, DXGI_FORMAT format, DX12Utils::ResourceType resourceType, const char* fileName, bool sourceIsSRGB, unsigned int size[3], const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState)
    {
        // Get the desired channel type
        DX12Utils::DXGI_FORMAT_Info formatInfo = DX12Utils::Get_DXGI_FORMAT_Info(format, Context::LogFn);
        DX12Utils::TextureCache::Type desiredChannelType = DX12Utils::TextureCache::Type::U8;
        if (formatInfo.channelType == DX12Utils::DXGI_FORMAT_Info::ChannelType::_uint8_t)
            desiredChannelType = DX12Utils::TextureCache::Type::U8;
        else if (formatInfo.channelType == DX12Utils::DXGI_FORMAT_Info::ChannelType::_float)
            desiredChannelType = DX12Utils::TextureCache::Type::F32;
        else
            return nullptr;

        if (resourceType == DX12Utils::ResourceType::Texture2D)
        {
            // Load the texture and convert as necessary
            DX12Utils::TextureCache::Texture texture = DX12Utils::TextureCache::GetAs(fileName, sourceIsSRGB, desiredChannelType, formatInfo.sRGB, formatInfo.channelCount);
            if (!texture.Valid())
                return nullptr;

            // store off image properties
            size[0] = texture.width;
            size[1] = texture.height;
            size[2] = 1;

            // make and return the texture
            return CreateManagedTexture(device, commandList, flags, format, size, 1, resourceType, texture.pixels.data(), debugName, desiredState);
        }
        else if (resourceType == DX12Utils::ResourceType::Texture2DArray ||
                 resourceType == DX12Utils::ResourceType::Texture3D ||
                 resourceType == DX12Utils::ResourceType::TextureCube)
        {
            static const char* c_cubeMapNames[] =
            {
                "Right",
                "Left",
                "Up",
                "Down",
                "Front",
                "Back"
            };

            bool useCubeMapNames = (resourceType == DX12Utils::ResourceType::TextureCube && strstr(fileName, "%s") != nullptr);
            bool hasPercentI = strstr(fileName, "%i") != nullptr;
            if (!useCubeMapNames && !hasPercentI)
                return nullptr;

            std::vector<DX12Utils::TextureCache::Texture> loadedTextureSlices;

            // Load multiple textures
            int textureIndex = -1;
            while (1)
            {
                textureIndex++;
                char indexedFileName[1024];

                if (useCubeMapNames)
                    sprintf_s(indexedFileName, fileName, c_cubeMapNames[textureIndex]);
                else
                    sprintf_s(indexedFileName, fileName, textureIndex);

                // Load the texture and convert as necessary
                DX12Utils::TextureCache::Texture loadedTextureSlice = DX12Utils::TextureCache::GetAs(indexedFileName, sourceIsSRGB, desiredChannelType, formatInfo.sRGB, formatInfo.channelCount);
                if (!loadedTextureSlice.Valid())
                {
                    if (textureIndex == 0)
                        return nullptr;
                    break;
                }

                // make sure the textures are the same size
                if (textureIndex > 0 && (loadedTextureSlice.width != loadedTextureSlices[0].width || loadedTextureSlice.height != loadedTextureSlices[0].height))
                    return nullptr;

                loadedTextureSlices.push_back(loadedTextureSlice);
            }

            // store the texture size
            size[0] = loadedTextureSlices[0].width;
            size[1] = loadedTextureSlices[0].height;
            size[2] = (unsigned int)loadedTextureSlices.size();

            // gather up all pixels into a contiguous chunk of memory
            std::vector<unsigned char> allPixels;
            for (const DX12Utils::TextureCache::Texture& texture : loadedTextureSlices)
                allPixels.insert(allPixels.end(), texture.pixels.begin(), texture.pixels.end());

            // make and return the texture
            return CreateManagedTexture(device, commandList, flags, format, size, 1, resourceType, allPixels.data(), debugName, desiredState);
        }
        else
            return nullptr;
    }

    void Context::UploadTextureData(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12Resource* texture, D3D12_RESOURCE_STATES textureState, const void* data, unsigned int unalignedPitch)
    {
        // Get information about the texture
        int alignedPitch = ALIGN(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT, unalignedPitch);
        D3D12_RESOURCE_DESC textureDesc = texture->GetDesc();

        // transition the resource to copy dest if it isn't already
        if (textureState != D3D12_RESOURCE_STATE_COPY_DEST)
        {
            D3D12_RESOURCE_BARRIER barrier;

            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = texture;
            barrier.Transition.StateBefore = textureState;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, &barrier);
        }

        // 3d textures do a single copy because it's a single sub resource.
        if (textureDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
        {
            // Get the upload buffer
            DX12Utils::UploadBufferTracker::UploadBufferTracker::Buffer* uploadBuffer = s_ubTracker.GetBuffer(device, alignedPitch * textureDesc.Height * textureDesc.DepthOrArraySize, Context::LogFn, false);

            // Map, copy, unmap
            {
                unsigned char* dest = nullptr;
                D3D12_RANGE readRange = { 0, 0 };
                HRESULT hr = uploadBuffer->buffer->Map(0, &readRange, (void**)&dest);
                if (FAILED(hr))
                {
                    Context::LogFn(LogLevel::Error, "Could not map upload buffer.");
                }
                else
                {
                    const unsigned char* src = (const unsigned char*)data;
                    for (int iz = 0; iz < textureDesc.DepthOrArraySize; ++iz)
                    {
                        for (int iy = 0; iy < (int)textureDesc.Height; ++iy)
                        {
                            memcpy(dest, src, unalignedPitch);
                            src += unalignedPitch;
                            dest += alignedPitch;
                        }
                    }

                    uploadBuffer->buffer->Unmap(0, nullptr);
                }
            }

            // copy the upload buffer into the texture
            {
                unsigned char layoutMem[sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)];
                D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutMem;
                device->GetCopyableFootprints(&textureDesc, 0, 1, 0, layout, nullptr, nullptr, nullptr);

                D3D12_TEXTURE_COPY_LOCATION src = {};
                src.pResource = uploadBuffer->buffer;
                src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                src.PlacedFootprint = *layout;

                D3D12_TEXTURE_COPY_LOCATION dest = {};
                dest.pResource = texture;
                dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                dest.SubresourceIndex = 0;

                commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
            }
        }
        // 2d array textures do a copy for each slice
        else if (textureDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
        {
            for (int iz = 0; iz < textureDesc.DepthOrArraySize; ++iz)
            {
                // Get the upload buffer
                DX12Utils::UploadBufferTracker::Buffer* uploadBuffer = s_ubTracker.GetBuffer(device, alignedPitch * textureDesc.Height, Context::LogFn, false);

                // Map, copy, unmap
                {
                    unsigned char* dest = nullptr;
                    D3D12_RANGE readRange = { 0, 0 };
                    HRESULT hr = uploadBuffer->buffer->Map(0, &readRange, (void**)&dest);
                    if (FAILED(hr))
                    {
                        Context::LogFn(LogLevel::Error, "Could not map upload buffer.");
                    }
                    else
                    {
                        const unsigned char* src = &((const unsigned char*)data)[unalignedPitch * textureDesc.Height * iz];
                        for (int iy = 0; iy < (int)textureDesc.Height; ++iy)
                        {
                            memcpy(dest, src, unalignedPitch);
                            src += unalignedPitch;
                            dest += alignedPitch;
                        }

                        uploadBuffer->buffer->Unmap(0, nullptr);
                    }
                }

                 // copy the upload buffer into the texture
                 {
                     unsigned char layoutMem[sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)];
                     D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutMem;
                     device->GetCopyableFootprints(&textureDesc, 0, 1, 0, layout, nullptr, nullptr, nullptr);

                     D3D12_TEXTURE_COPY_LOCATION src = {};
                     src.pResource = uploadBuffer->buffer;
                     src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                     src.PlacedFootprint = *layout;

                     D3D12_TEXTURE_COPY_LOCATION dest = {};
                     dest.pResource = texture;
                     dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                     dest.SubresourceIndex = iz;

                     commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
                 }
            }
        }
        else
        {
            Context::LogFn(LogLevel::Error, "Unhandled texture dimension.");
        }

        // transition the resource back to what it was
        if (textureState != D3D12_RESOURCE_STATE_COPY_DEST)
        {
            D3D12_RESOURCE_BARRIER barrier;

            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = texture;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.StateAfter = textureState;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, &barrier);
        }
    }

    void Context::UploadBufferData(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12Resource* buffer, D3D12_RESOURCE_STATES bufferState, const void* data, unsigned int dataSize)
    {
        // Get the upload buffer
        DX12Utils::UploadBufferTracker::UploadBufferTracker::Buffer* uploadBuffer = s_ubTracker.GetBuffer(device, dataSize, Context::LogFn, false);

        // copy cpu data to the upload buffer
        {
            void* start = nullptr;
            HRESULT hr = uploadBuffer->buffer->Map(0, nullptr, reinterpret_cast<void**>(&start));
            if(hr)
            {
                Context::LogFn(LogLevel::Error, "Could not map upload buffer");
                return;
            }

            memcpy(start, data, dataSize);

            uploadBuffer->buffer->Unmap(0, nullptr);
        }

        // transition the resource to copy dest if it isn't already
        if (bufferState != D3D12_RESOURCE_STATE_COPY_DEST)
        {
            D3D12_RESOURCE_BARRIER barrier;

            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = buffer;
            barrier.Transition.StateBefore = bufferState;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, &barrier);
        }

        // copy the resource
        commandList->CopyResource(buffer, uploadBuffer->buffer);

        // transition the resource back to what it was
        if (bufferState != D3D12_RESOURCE_STATE_COPY_DEST)
        {
            D3D12_RESOURCE_BARRIER barrier;

            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = buffer;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.StateAfter = bufferState;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, &barrier);
        }
    }

    int Context::GetRTV(ID3D12Device* device, ID3D12Resource* resource, DXGI_FORMAT format, D3D12_RTV_DIMENSION dimension, int arrayIndex, int mipIndex, const char* debugName)
    {
        // Make the key
        DX12Utils::SubResourceHeapAllocationInfo key;
        key.resource = resource;
        key.arrayIndex = arrayIndex;
        key.mipIndex = mipIndex;

        // If it already exists, use it
        auto it = m_internal.m_RTVCache.find(key);
        if (it != m_internal.m_RTVCache.end())
            return it->second;

        // Allocate an RTV index
        int rtvIndex = -1;
        if (!s_heapAllocationTrackerRTV.Allocate(rtvIndex, debugName))
            return -1;

        // Create the RTV
        if (!DX12Utils::CreateRTV(device, resource, s_heapAllocationTrackerRTV.GetCPUHandle(rtvIndex), format, dimension, arrayIndex, mipIndex))
        {
            s_heapAllocationTrackerRTV.Free(rtvIndex);
            return -1;
        }

        // store the result
        m_internal.m_RTVCache[key] = rtvIndex;
        return rtvIndex;
    }

    int Context::GetDSV(ID3D12Device* device, ID3D12Resource* resource, DXGI_FORMAT format, D3D12_DSV_DIMENSION dimension, int arrayIndex, int mipIndex, const char* debugName)
    {
	    // Make the key
        DX12Utils::SubResourceHeapAllocationInfo key;
        key.resource = resource;
        key.arrayIndex = arrayIndex;
        key.mipIndex = mipIndex;

	    // If it already exists, use it
	    auto it = m_internal.m_DSVCache.find(key);
	    if (it != m_internal.m_DSVCache.end())
            return it->second;

        // Allocate a DSV index
        int dsvIndex = -1;
        if (!s_heapAllocationTrackerDSV.Allocate(dsvIndex, debugName))
            return -1;

        // Create the DSV
        if (!DX12Utils::CreateDSV(device, resource, s_heapAllocationTrackerDSV.GetCPUHandle(dsvIndex), format, dimension, arrayIndex, mipIndex))
        {
            s_heapAllocationTrackerDSV.Free(dsvIndex);
            return -1;
        }

        // store the result
        m_internal.m_DSVCache[key] = dsvIndex;
        return dsvIndex;
    }

    const ProfileEntry* Context::ReadbackProfileData(ID3D12CommandQueue* commandQueue, int& numItems)
    {
        numItems = 0;

        if (!m_profile || !m_internal.m_TimestampReadbackBuffer)
            return nullptr;

        uint64_t GPUFrequency;
        commandQueue->GetTimestampFrequency(&GPUFrequency);
        double GPUTickDelta = 1.0 / static_cast<double>(GPUFrequency);

        D3D12_RANGE range;
        range.Begin = 0;
        range.End = ((9 + 1) * 2) * sizeof(uint64_t);

        uint64_t* timeStampBuffer = nullptr;
        m_internal.m_TimestampReadbackBuffer->Map(0, &range, (void**)&timeStampBuffer);

        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // compute shader: Histogram_InitMinMaxValue
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // compute shader: Histogram_InitCounts
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // compute shader: Display
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // compute shader: Histogram_MakeMinMaxValue
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // compute shader: Histogram_MakeCounts
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // compute shader: Histogram_MakeGraph
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // compute shader: DFT_DFT
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // compute shader: DFT_Normalize
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+2] - timeStampBuffer[numItems*2+1])); numItems++; // compute shader: MakeCombinedOutput
        m_profileData[numItems].m_gpu = float(GPUTickDelta * double(timeStampBuffer[numItems*2+1] - timeStampBuffer[0])); numItems++; // GPU total

        D3D12_RANGE emptyRange = {};
        m_internal.m_TimestampReadbackBuffer->Unmap(0, &emptyRange);

        return m_profileData;
    }

    Context::~Context()
    {
        for (const auto& pair : m_internal.m_RTVCache)
            s_heapAllocationTrackerRTV.Free(pair.second);
        m_internal.m_RTVCache.clear();

        for (const auto& pair : m_internal.m_DSVCache)
            s_heapAllocationTrackerDSV.Free(pair.second);
        m_internal.m_DSVCache.clear();

        for (ID3D12Resource* resource : m_internal.m_managedResources)
            resource->Release();
        m_internal.m_managedResources.clear();

        if(m_internal.m_TimestampQueryHeap)
        {
            m_internal.m_TimestampQueryHeap->Release();
            m_internal.m_TimestampQueryHeap = nullptr;
        }

        if(m_internal.m_TimestampReadbackBuffer)
        {
            m_internal.m_TimestampReadbackBuffer->Release();
            m_internal.m_TimestampReadbackBuffer = nullptr;
        }

        if(m_output.texture_Output)
        {
            s_delayedRelease.Add(m_output.texture_Output);
            m_output.texture_Output = nullptr;
        }

        if(m_output.texture_OutputF)
        {
            s_delayedRelease.Add(m_output.texture_OutputF);
            m_output.texture_OutputF = nullptr;
        }

        if(m_output.texture_CombinedOutput)
        {
            s_delayedRelease.Add(m_output.texture_CombinedOutput);
            m_output.texture_CombinedOutput = nullptr;
        }

        if(m_output.buffer_Histogram_Counts)
        {
            s_delayedRelease.Add(m_output.buffer_Histogram_Counts);
            m_output.buffer_Histogram_Counts = nullptr;
        }

        if(m_output.texture_Histogram_Graph)
        {
            s_delayedRelease.Add(m_output.texture_Histogram_Graph);
            m_output.texture_Histogram_Graph = nullptr;
        }

        if(m_output.buffer_Histogram_MinMaxValue)
        {
            s_delayedRelease.Add(m_output.buffer_Histogram_MinMaxValue);
            m_output.buffer_Histogram_MinMaxValue = nullptr;
        }

        if(m_output.texture_DFT_DFTMagnitude)
        {
            s_delayedRelease.Add(m_output.texture_DFT_DFTMagnitude);
            m_output.texture_DFT_DFTMagnitude = nullptr;
        }

        if(m_internal.buffer_DFT_MaxMagnitude)
        {
            s_delayedRelease.Add(m_internal.buffer_DFT_MaxMagnitude);
            m_internal.buffer_DFT_MaxMagnitude = nullptr;
        }

        if(m_internal.texture__loadedTexture_0)
        {
            s_delayedRelease.Add(m_internal.texture__loadedTexture_0);
            m_internal.texture__loadedTexture_0 = nullptr;
        }

        if(m_internal.texture__loadedTexture_1)
        {
            s_delayedRelease.Add(m_internal.texture__loadedTexture_1);
            m_internal.texture__loadedTexture_1 = nullptr;
        }

        if(m_internal.texture__loadedTexture_2)
        {
            s_delayedRelease.Add(m_internal.texture__loadedTexture_2);
            m_internal.texture__loadedTexture_2 = nullptr;
        }

        if(m_internal.texture__loadedTexture_3)
        {
            s_delayedRelease.Add(m_internal.texture__loadedTexture_3);
            m_internal.texture__loadedTexture_3 = nullptr;
        }

        // _DisplayCSCB
        if (m_internal.constantBuffer__DisplayCSCB)
        {
            s_delayedRelease.Add(m_internal.constantBuffer__DisplayCSCB);
            m_internal.constantBuffer__DisplayCSCB = nullptr;
        }

        // _Histogram_MakeCountsCSCB
        if (m_internal.constantBuffer__Histogram_MakeCountsCSCB)
        {
            s_delayedRelease.Add(m_internal.constantBuffer__Histogram_MakeCountsCSCB);
            m_internal.constantBuffer__Histogram_MakeCountsCSCB = nullptr;
        }

        // _Histogram_MakeGraphCSCB
        if (m_internal.constantBuffer__Histogram_MakeGraphCSCB)
        {
            s_delayedRelease.Add(m_internal.constantBuffer__Histogram_MakeGraphCSCB);
            m_internal.constantBuffer__Histogram_MakeGraphCSCB = nullptr;
        }

        // _Histogram_MakeMinMaxValueCSCB
        if (m_internal.constantBuffer__Histogram_MakeMinMaxValueCSCB)
        {
            s_delayedRelease.Add(m_internal.constantBuffer__Histogram_MakeMinMaxValueCSCB);
            m_internal.constantBuffer__Histogram_MakeMinMaxValueCSCB = nullptr;
        }

        // _DFT_DFTCSCB
        if (m_internal.constantBuffer__DFT_DFTCSCB)
        {
            s_delayedRelease.Add(m_internal.constantBuffer__DFT_DFTCSCB);
            m_internal.constantBuffer__DFT_DFTCSCB = nullptr;
        }

        // _DFT_NormalizeCSCB
        if (m_internal.constantBuffer__DFT_NormalizeCSCB)
        {
            s_delayedRelease.Add(m_internal.constantBuffer__DFT_NormalizeCSCB);
            m_internal.constantBuffer__DFT_NormalizeCSCB = nullptr;
        }
    }

    void Execute(Context* context, ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
    {
        // reset the timer index
        s_timerIndex = 0;

        ScopedPerfEvent scopedPerf("BNOctaves", commandList, 27);

        std::chrono::high_resolution_clock::time_point startPointCPUTechnique;
        if(context->m_profile)
        {
            startPointCPUTechnique = std::chrono::high_resolution_clock::now();
            if(context->m_internal.m_TimestampQueryHeap == nullptr)
            {
                D3D12_QUERY_HEAP_DESC QueryHeapDesc;
                QueryHeapDesc.Count = (9+1) * 2;
                QueryHeapDesc.NodeMask = 1;
                QueryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
                device->CreateQueryHeap(&QueryHeapDesc, IID_PPV_ARGS(&context->m_internal.m_TimestampQueryHeap));
                if (c_debugNames)
                    context->m_internal.m_TimestampQueryHeap->SetName(L"BNOctaves Time Stamp Query Heap");

                context->m_internal.m_TimestampReadbackBuffer = DX12Utils::CreateBuffer(device, sizeof(uint64_t) * (9+1) * 2, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_READBACK, (c_debugNames ? L"BNOctaves Time Stamp Query Heap" : nullptr), nullptr);
            }
            commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
        }

        // Make sure internally owned resources are created and are the right size and format
        context->EnsureResourcesCreated(device, commandList);

        // set the heaps
        ID3D12DescriptorHeap* heaps[] =
        {
            s_srvHeap.m_heap,
        };
        commandList->SetDescriptorHeaps(_countof(heaps), heaps);

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[1];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].Transition.pResource = context->m_output.buffer_Histogram_MinMaxValue;
            barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, barriers);
        }

        // Compute Shader: Histogram_InitMinMaxValue
        {
            ScopedPerfEvent scopedPerf("Compute Shader: Histogram_InitMinMaxValue", commandList, 11);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetComputeRootSignature(ContextInternal::computeShader_Histogram_InitMinMaxValue_rootSig);
            commandList->SetPipelineState(ContextInternal::computeShader_Histogram_InitMinMaxValue_pso);

            DX12Utils::ResourceDescriptor descriptors[] = {
                { context->m_output.buffer_Histogram_MinMaxValue, context->m_output.buffer_Histogram_MinMaxValue_format, DX12Utils::AccessType::UAV, DX12Utils::ResourceType::Buffer, false, context->m_output.buffer_Histogram_MinMaxValue_stride, context->m_output.buffer_Histogram_MinMaxValue_count, 0 }
            };

            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable = GetDescriptorTable(device, s_srvHeap, descriptors, 1, Context::LogFn);
            commandList->SetComputeRootDescriptorTable(0, descriptorTable);

            unsigned int baseDispatchSize[3] = { 1, 1, 1 };

            unsigned int dispatchSize[3] = {
                (((baseDispatchSize[0] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[1] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[2] + 0) * 1) / 1 + 0 + 1 - 1) / 1
            };

            commandList->Dispatch(dispatchSize[0], dispatchSize[1], dispatchSize[2]);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "Histogram_InitMinMaxValue";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[1];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].Transition.pResource = context->m_output.buffer_Histogram_Counts;
            barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(1, barriers);
        }

        // Compute Shader: Histogram_InitCounts
        {
            ScopedPerfEvent scopedPerf("Compute Shader: Histogram_InitCounts", commandList, 12);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetComputeRootSignature(ContextInternal::computeShader_Histogram_InitCounts_rootSig);
            commandList->SetPipelineState(ContextInternal::computeShader_Histogram_InitCounts_pso);

            DX12Utils::ResourceDescriptor descriptors[] = {
                { context->m_output.buffer_Histogram_Counts, context->m_output.buffer_Histogram_Counts_format, DX12Utils::AccessType::UAV, DX12Utils::ResourceType::Buffer, false, context->m_output.buffer_Histogram_Counts_stride, context->m_output.buffer_Histogram_Counts_count, 0 }
            };

            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable = GetDescriptorTable(device, s_srvHeap, descriptors, 1, Context::LogFn);
            commandList->SetComputeRootDescriptorTable(0, descriptorTable);

            unsigned int baseDispatchSize[3] = { context->m_output.buffer_Histogram_Counts_count, 1, 1 };

            unsigned int dispatchSize[3] = {
                (((baseDispatchSize[0] + 0) * 1) / 1 + 0 + 64 - 1) / 64,
                (((baseDispatchSize[1] + 0) * 1) / 1 + 0 + 1 - 1) / 1,
                (((baseDispatchSize[2] + 0) * 1) / 1 + 0 + 1 - 1) / 1
            };

            commandList->Dispatch(dispatchSize[0], dispatchSize[1], dispatchSize[2]);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "Histogram_InitCounts";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Shader Constants: _DisplayCSCB
        {
            context->m_internal.constantBuffer__DisplayCSCB_cpu.DifferentNoisePerOctave = context->m_input.variable_DifferentNoisePerOctave;
            context->m_internal.constantBuffer__DisplayCSCB_cpu.NoiseType = (int)context->m_input.variable_NoiseType;
            context->m_internal.constantBuffer__DisplayCSCB_cpu.NumberOfOctaves = context->m_input.variable_NumberOfOctaves;
            context->m_internal.constantBuffer__DisplayCSCB_cpu.PerlinCellSize = context->m_input.variable_PerlinCellSize;
            context->m_internal.constantBuffer__DisplayCSCB_cpu.PerlinMinMax = context->m_input.variable_PerlinMinMax;
            context->m_internal.constantBuffer__DisplayCSCB_cpu.RNGSeed = context->m_input.variable_RNGSeed;
            DX12Utils::CopyConstantsCPUToGPU(s_ubTracker, device, commandList, context->m_internal.constantBuffer__DisplayCSCB, context->m_internal.constantBuffer__DisplayCSCB_cpu, Context::LogFn);
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[2];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].UAV.pResource = context->m_output.texture_Output;

            barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[1].Transition.pResource = context->m_output.texture_OutputF;
            barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(2, barriers);
        }

        // Compute Shader: Display
        {
            ScopedPerfEvent scopedPerf("Compute Shader: Display", commandList, 0);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetComputeRootSignature(ContextInternal::computeShader_Display_rootSig);
            commandList->SetPipelineState(ContextInternal::computeShader_Display_pso);

            DX12Utils::ResourceDescriptor descriptors[] = {
                { context->m_output.texture_Output, context->m_output.texture_Output_format, DX12Utils::AccessType::UAV, DX12Utils::ResourceType::Texture2D, false, 0, 0, 0 },
                { context->m_output.texture_OutputF, context->m_output.texture_OutputF_format, DX12Utils::AccessType::UAV, DX12Utils::ResourceType::Texture2D, false, 0, 0, 0 },
                { context->m_internal.texture__loadedTexture_0, context->m_internal.texture__loadedTexture_0_format, DX12Utils::AccessType::SRV, DX12Utils::ResourceType::Texture2DArray, false, 0, context->m_internal.texture__loadedTexture_0_size[2], 0 },
                { context->m_internal.texture__loadedTexture_1, context->m_internal.texture__loadedTexture_1_format, DX12Utils::AccessType::SRV, DX12Utils::ResourceType::Texture2DArray, false, 0, context->m_internal.texture__loadedTexture_1_size[2], 0 },
                { context->m_internal.texture__loadedTexture_2, context->m_internal.texture__loadedTexture_2_format, DX12Utils::AccessType::SRV, DX12Utils::ResourceType::Texture2DArray, false, 0, context->m_internal.texture__loadedTexture_2_size[2], 0 },
                { context->m_internal.texture__loadedTexture_3, context->m_internal.texture__loadedTexture_3_format, DX12Utils::AccessType::SRV, DX12Utils::ResourceType::Texture2DArray, false, 0, context->m_internal.texture__loadedTexture_3_size[2], 0 },
                { context->m_internal.constantBuffer__DisplayCSCB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0 }
            };

            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable = GetDescriptorTable(device, s_srvHeap, descriptors, 7, Context::LogFn);
            commandList->SetComputeRootDescriptorTable(0, descriptorTable);

            unsigned int baseDispatchSize[3] = {
                context->m_output.texture_Output_size[0],
                context->m_output.texture_Output_size[1],
                context->m_output.texture_Output_size[2]
            };

            unsigned int dispatchSize[3] = {
                (((baseDispatchSize[0] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[1] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[2] + 0) * 1) / 1 + 0 + 1 - 1) / 1
            };

            commandList->Dispatch(dispatchSize[0], dispatchSize[1], dispatchSize[2]);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "Display";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Shader Constants: _Histogram_MakeCountsCSCB
        {
            context->m_internal.constantBuffer__Histogram_MakeCountsCSCB_cpu.Histogram_NumBuckets = context->m_input.variable_Histogram_NumBuckets;
            context->m_internal.constantBuffer__Histogram_MakeCountsCSCB_cpu.Histogram_ZeroMinMaxBucket = context->m_input.variable_Histogram_ZeroMinMaxBucket;
            DX12Utils::CopyConstantsCPUToGPU(s_ubTracker, device, commandList, context->m_internal.constantBuffer__Histogram_MakeCountsCSCB, context->m_internal.constantBuffer__Histogram_MakeCountsCSCB_cpu, Context::LogFn);
        }

        // Shader Constants: _Histogram_MakeGraphCSCB
        {
            context->m_internal.constantBuffer__Histogram_MakeGraphCSCB_cpu.Histogram_GraphSize = context->m_input.variable_Histogram_GraphSize;
            context->m_internal.constantBuffer__Histogram_MakeGraphCSCB_cpu.Histogram_NumBuckets = context->m_input.variable_Histogram_NumBuckets;
            DX12Utils::CopyConstantsCPUToGPU(s_ubTracker, device, commandList, context->m_internal.constantBuffer__Histogram_MakeGraphCSCB, context->m_internal.constantBuffer__Histogram_MakeGraphCSCB_cpu, Context::LogFn);
        }

        // Shader Constants: _Histogram_MakeMinMaxValueCSCB
        {
            context->m_internal.constantBuffer__Histogram_MakeMinMaxValueCSCB_cpu.Histogram_AutoXAxisRange = context->m_input.variable_Histogram_AutoXAxisRange;
            context->m_internal.constantBuffer__Histogram_MakeMinMaxValueCSCB_cpu.Histogram_XAxisRange = context->m_input.variable_Histogram_XAxisRange;
            DX12Utils::CopyConstantsCPUToGPU(s_ubTracker, device, commandList, context->m_internal.constantBuffer__Histogram_MakeMinMaxValueCSCB, context->m_internal.constantBuffer__Histogram_MakeMinMaxValueCSCB_cpu, Context::LogFn);
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[2];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].Transition.pResource = context->m_output.texture_OutputF;
            barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            barriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[1].UAV.pResource = context->m_output.buffer_Histogram_MinMaxValue;

            commandList->ResourceBarrier(2, barriers);
        }

        // Compute Shader: Histogram_MakeMinMaxValue
        {
            ScopedPerfEvent scopedPerf("Compute Shader: Histogram_MakeMinMaxValue", commandList, 10);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetComputeRootSignature(ContextInternal::computeShader_Histogram_MakeMinMaxValue_rootSig);
            commandList->SetPipelineState(ContextInternal::computeShader_Histogram_MakeMinMaxValue_pso);

            DX12Utils::ResourceDescriptor descriptors[] = {
                { context->m_output.texture_OutputF, context->m_output.texture_OutputF_format, DX12Utils::AccessType::SRV, DX12Utils::ResourceType::Texture2D, false, 0, 0, 0 },
                { context->m_output.buffer_Histogram_MinMaxValue, context->m_output.buffer_Histogram_MinMaxValue_format, DX12Utils::AccessType::UAV, DX12Utils::ResourceType::Buffer, false, context->m_output.buffer_Histogram_MinMaxValue_stride, context->m_output.buffer_Histogram_MinMaxValue_count, 0 },
                { context->m_internal.constantBuffer__Histogram_MakeMinMaxValueCSCB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0 }
            };

            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable = GetDescriptorTable(device, s_srvHeap, descriptors, 3, Context::LogFn);
            commandList->SetComputeRootDescriptorTable(0, descriptorTable);

            unsigned int baseDispatchSize[3] = {
                context->m_output.texture_OutputF_size[0],
                context->m_output.texture_OutputF_size[1],
                context->m_output.texture_OutputF_size[2]
            };

            unsigned int dispatchSize[3] = {
                (((baseDispatchSize[0] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[1] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[2] + 0) * 1) / 1 + 0 + 1 - 1) / 1
            };

            commandList->Dispatch(dispatchSize[0], dispatchSize[1], dispatchSize[2]);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "Histogram_MakeMinMaxValue";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[2];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].UAV.pResource = context->m_output.buffer_Histogram_Counts;

            barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[1].Transition.pResource = context->m_output.buffer_Histogram_MinMaxValue;
            barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(2, barriers);
        }

        // Compute Shader: Histogram_MakeCounts
        {
            ScopedPerfEvent scopedPerf("Compute Shader: Histogram_MakeCounts", commandList, 5);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetComputeRootSignature(ContextInternal::computeShader_Histogram_MakeCounts_rootSig);
            commandList->SetPipelineState(ContextInternal::computeShader_Histogram_MakeCounts_pso);

            DX12Utils::ResourceDescriptor descriptors[] = {
                { context->m_output.texture_OutputF, context->m_output.texture_OutputF_format, DX12Utils::AccessType::SRV, DX12Utils::ResourceType::Texture2D, false, 0, 0, 0 },
                { context->m_output.buffer_Histogram_MinMaxValue, context->m_output.buffer_Histogram_MinMaxValue_format, DX12Utils::AccessType::SRV, DX12Utils::ResourceType::Buffer, false, context->m_output.buffer_Histogram_MinMaxValue_stride, context->m_output.buffer_Histogram_MinMaxValue_count, 0 },
                { context->m_output.buffer_Histogram_Counts, context->m_output.buffer_Histogram_Counts_format, DX12Utils::AccessType::UAV, DX12Utils::ResourceType::Buffer, false, context->m_output.buffer_Histogram_Counts_stride, context->m_output.buffer_Histogram_Counts_count, 0 },
                { context->m_internal.constantBuffer__Histogram_MakeCountsCSCB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0 }
            };

            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable = GetDescriptorTable(device, s_srvHeap, descriptors, 4, Context::LogFn);
            commandList->SetComputeRootDescriptorTable(0, descriptorTable);

            unsigned int baseDispatchSize[3] = {
                context->m_output.texture_OutputF_size[0],
                context->m_output.texture_OutputF_size[1],
                context->m_output.texture_OutputF_size[2]
            };

            unsigned int dispatchSize[3] = {
                (((baseDispatchSize[0] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[1] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[2] + 0) * 1) / 1 + 0 + 1 - 1) / 1
            };

            commandList->Dispatch(dispatchSize[0], dispatchSize[1], dispatchSize[2]);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "Histogram_MakeCounts";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[2];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].Transition.pResource = context->m_output.buffer_Histogram_Counts;
            barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[1].Transition.pResource = context->m_output.texture_Histogram_Graph;
            barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(2, barriers);
        }

        // Compute Shader: Histogram_MakeGraph
        {
            ScopedPerfEvent scopedPerf("Compute Shader: Histogram_MakeGraph", commandList, 8);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetComputeRootSignature(ContextInternal::computeShader_Histogram_MakeGraph_rootSig);
            commandList->SetPipelineState(ContextInternal::computeShader_Histogram_MakeGraph_pso);

            DX12Utils::ResourceDescriptor descriptors[] = {
                { context->m_output.buffer_Histogram_Counts, context->m_output.buffer_Histogram_Counts_format, DX12Utils::AccessType::SRV, DX12Utils::ResourceType::Buffer, false, context->m_output.buffer_Histogram_Counts_stride, context->m_output.buffer_Histogram_Counts_count, 0 },
                { context->m_output.texture_Histogram_Graph, context->m_output.texture_Histogram_Graph_format, DX12Utils::AccessType::UAV, DX12Utils::ResourceType::Texture2D, false, 0, 0, 0 },
                { context->m_internal.constantBuffer__Histogram_MakeGraphCSCB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0 }
            };

            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable = GetDescriptorTable(device, s_srvHeap, descriptors, 3, Context::LogFn);
            commandList->SetComputeRootDescriptorTable(0, descriptorTable);

            unsigned int baseDispatchSize[3] = {
                context->m_output.texture_Histogram_Graph_size[0],
                context->m_output.texture_Histogram_Graph_size[1],
                context->m_output.texture_Histogram_Graph_size[2]
            };

            unsigned int dispatchSize[3] = {
                (((baseDispatchSize[0] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[1] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[2] + 0) * 1) / 1 + 0 + 1 - 1) / 1
            };

            commandList->Dispatch(dispatchSize[0], dispatchSize[1], dispatchSize[2]);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "Histogram_MakeGraph";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Shader Constants: _DFT_DFTCSCB
        {
            context->m_internal.constantBuffer__DFT_DFTCSCB_cpu.DFT_RemoveDC = context->m_input.variable_DFT_RemoveDC;
            DX12Utils::CopyConstantsCPUToGPU(s_ubTracker, device, commandList, context->m_internal.constantBuffer__DFT_DFTCSCB, context->m_internal.constantBuffer__DFT_DFTCSCB_cpu, Context::LogFn);
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[2];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].Transition.pResource = context->m_output.texture_DFT_DFTMagnitude;
            barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[1].Transition.pResource = context->m_internal.buffer_DFT_MaxMagnitude;
            barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(2, barriers);
        }

        // Compute Shader: DFT_DFT
        {
            ScopedPerfEvent scopedPerf("Compute Shader: DFT_DFT", commandList, 14);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetComputeRootSignature(ContextInternal::computeShader_DFT_DFT_rootSig);
            commandList->SetPipelineState(ContextInternal::computeShader_DFT_DFT_pso);

            DX12Utils::ResourceDescriptor descriptors[] = {
                { context->m_output.texture_OutputF, context->m_output.texture_OutputF_format, DX12Utils::AccessType::SRV, DX12Utils::ResourceType::Texture2D, false, 0, 0, 0 },
                { context->m_output.texture_DFT_DFTMagnitude, context->m_output.texture_DFT_DFTMagnitude_format, DX12Utils::AccessType::UAV, DX12Utils::ResourceType::Texture2D, false, 0, 0, 0 },
                { context->m_internal.buffer_DFT_MaxMagnitude, context->m_internal.buffer_DFT_MaxMagnitude_format, DX12Utils::AccessType::UAV, DX12Utils::ResourceType::Buffer, false, context->m_internal.buffer_DFT_MaxMagnitude_stride, context->m_internal.buffer_DFT_MaxMagnitude_count, 0 },
                { context->m_internal.constantBuffer__DFT_DFTCSCB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0 }
            };

            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable = GetDescriptorTable(device, s_srvHeap, descriptors, 4, Context::LogFn);
            commandList->SetComputeRootDescriptorTable(0, descriptorTable);

            unsigned int baseDispatchSize[3] = {
                context->m_output.texture_OutputF_size[0],
                context->m_output.texture_OutputF_size[1],
                context->m_output.texture_OutputF_size[2]
            };

            unsigned int dispatchSize[3] = {
                (((baseDispatchSize[0] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[1] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[2] + 0) * 1) / 1 + 0 + 1 - 1) / 1
            };

            commandList->Dispatch(dispatchSize[0], dispatchSize[1], dispatchSize[2]);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "DFT_DFT";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Shader Constants: _DFT_NormalizeCSCB
        {
            context->m_internal.constantBuffer__DFT_NormalizeCSCB_cpu.DFT_LogSpaceMagnitude = context->m_input.variable_DFT_LogSpaceMagnitude;
            DX12Utils::CopyConstantsCPUToGPU(s_ubTracker, device, commandList, context->m_internal.constantBuffer__DFT_NormalizeCSCB, context->m_internal.constantBuffer__DFT_NormalizeCSCB_cpu, Context::LogFn);
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[2];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].UAV.pResource = context->m_output.texture_DFT_DFTMagnitude;

            barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[1].Transition.pResource = context->m_internal.buffer_DFT_MaxMagnitude;
            barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(2, barriers);
        }

        // Compute Shader: DFT_Normalize
        {
            ScopedPerfEvent scopedPerf("Compute Shader: DFT_Normalize", commandList, 16);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetComputeRootSignature(ContextInternal::computeShader_DFT_Normalize_rootSig);
            commandList->SetPipelineState(ContextInternal::computeShader_DFT_Normalize_pso);

            DX12Utils::ResourceDescriptor descriptors[] = {
                { context->m_output.texture_DFT_DFTMagnitude, context->m_output.texture_DFT_DFTMagnitude_format, DX12Utils::AccessType::UAV, DX12Utils::ResourceType::Texture2D, false, 0, 0, 0 },
                { context->m_internal.buffer_DFT_MaxMagnitude, context->m_internal.buffer_DFT_MaxMagnitude_format, DX12Utils::AccessType::SRV, DX12Utils::ResourceType::Buffer, false, context->m_internal.buffer_DFT_MaxMagnitude_stride, context->m_internal.buffer_DFT_MaxMagnitude_count, 0 },
                { context->m_internal.constantBuffer__DFT_NormalizeCSCB, DXGI_FORMAT_UNKNOWN, DX12Utils::AccessType::CBV, DX12Utils::ResourceType::Buffer, false, 256, 1, 0 }
            };

            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable = GetDescriptorTable(device, s_srvHeap, descriptors, 3, Context::LogFn);
            commandList->SetComputeRootDescriptorTable(0, descriptorTable);

            unsigned int baseDispatchSize[3] = {
                context->m_output.texture_DFT_DFTMagnitude_size[0],
                context->m_output.texture_DFT_DFTMagnitude_size[1],
                context->m_output.texture_DFT_DFTMagnitude_size[2]
            };

            unsigned int dispatchSize[3] = {
                (((baseDispatchSize[0] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[1] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[2] + 0) * 1) / 1 + 0 + 1 - 1) / 1
            };

            commandList->Dispatch(dispatchSize[0], dispatchSize[1], dispatchSize[2]);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "DFT_Normalize";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        // Transition resources for the next action
        {
            D3D12_RESOURCE_BARRIER barriers[3];

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[0].UAV.pResource = context->m_output.texture_CombinedOutput;

            barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[1].Transition.pResource = context->m_output.texture_Histogram_Graph;
            barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            barriers[2].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[2].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barriers[2].Transition.pResource = context->m_output.texture_DFT_DFTMagnitude;
            barriers[2].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barriers[2].Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            barriers[2].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            commandList->ResourceBarrier(3, barriers);
        }

        // Compute Shader: MakeCombinedOutput
        {
            ScopedPerfEvent scopedPerf("Compute Shader: MakeCombinedOutput", commandList, 4);
            std::chrono::high_resolution_clock::time_point startPointCPU;
            if(context->m_profile)
            {
                startPointCPU = std::chrono::high_resolution_clock::now();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }

            commandList->SetComputeRootSignature(ContextInternal::computeShader_MakeCombinedOutput_rootSig);
            commandList->SetPipelineState(ContextInternal::computeShader_MakeCombinedOutput_pso);

            DX12Utils::ResourceDescriptor descriptors[] = {
                { context->m_output.texture_CombinedOutput, context->m_output.texture_CombinedOutput_format, DX12Utils::AccessType::UAV, DX12Utils::ResourceType::Texture2D, false, 0, 0, 0 },
                { context->m_output.texture_OutputF, context->m_output.texture_OutputF_format, DX12Utils::AccessType::SRV, DX12Utils::ResourceType::Texture2D, false, 0, 0, 0 },
                { context->m_output.texture_Histogram_Graph, context->m_output.texture_Histogram_Graph_format, DX12Utils::AccessType::SRV, DX12Utils::ResourceType::Texture2D, false, 0, 0, 0 },
                { context->m_output.texture_DFT_DFTMagnitude, context->m_output.texture_DFT_DFTMagnitude_format, DX12Utils::AccessType::SRV, DX12Utils::ResourceType::Texture2D, false, 0, 0, 0 }
            };

            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable = GetDescriptorTable(device, s_srvHeap, descriptors, 4, Context::LogFn);
            commandList->SetComputeRootDescriptorTable(0, descriptorTable);

            unsigned int baseDispatchSize[3] = {
                context->m_output.texture_CombinedOutput_size[0],
                context->m_output.texture_CombinedOutput_size[1],
                context->m_output.texture_CombinedOutput_size[2]
            };

            unsigned int dispatchSize[3] = {
                (((baseDispatchSize[0] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[1] + 0) * 1) / 1 + 0 + 8 - 1) / 8,
                (((baseDispatchSize[2] + 0) * 1) / 1 + 0 + 1 - 1) / 1
            };

            commandList->Dispatch(dispatchSize[0], dispatchSize[1], dispatchSize[2]);

            if(context->m_profile)
            {
                context->m_profileData[(s_timerIndex-1)/2].m_label = "MakeCombinedOutput";
                context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPU).count();
                commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            }
        }

        if(context->m_profile)
        {
            context->m_profileData[(s_timerIndex-1)/2].m_label = "Total";
            context->m_profileData[(s_timerIndex-1)/2].m_cpu = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - startPointCPUTechnique).count();
            commandList->EndQuery(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, s_timerIndex++);
            commandList->ResolveQueryData(context->m_internal.m_TimestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0, s_timerIndex, context->m_internal.m_TimestampReadbackBuffer, 0);
        }
    }

    void Context::EnsureResourcesCreated(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
    {
        bool dirty = false;

        // Output
        {
            unsigned int baseSize[3] = { 1, 1, 1 };

            unsigned int desiredSize[3] = {
                ((baseSize[0] + 0) * 128) / 1 + 0,
                ((baseSize[1] + 0) * 128) / 1 + 0,
                ((baseSize[2] + 0) * 1) / 1 + 0
            };

            static const unsigned int desiredNumMips = 1;

            DXGI_FORMAT desiredFormat = DXGI_FORMAT_R8_UNORM;

            if(!m_output.texture_Output ||
               m_output.texture_Output_size[0] != desiredSize[0] ||
               m_output.texture_Output_size[1] != desiredSize[1] ||
               m_output.texture_Output_size[2] != desiredSize[2] ||
               m_output.texture_Output_numMips != desiredNumMips ||
               m_output.texture_Output_format != desiredFormat)
            {
                dirty = true;
                if(m_output.texture_Output)
                    s_delayedRelease.Add(m_output.texture_Output);

                m_output.texture_Output = DX12Utils::CreateTexture(device, desiredSize, desiredNumMips, desiredFormat, m_output.texture_Output_flags, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, DX12Utils::ResourceType::Texture2D, (c_debugNames ? L"Output" : nullptr), Context::LogFn);
                m_output.texture_Output_size[0] = desiredSize[0];
                m_output.texture_Output_size[1] = desiredSize[1];
                m_output.texture_Output_size[2] = desiredSize[2];
                m_output.texture_Output_numMips = desiredNumMips;
                m_output.texture_Output_format = desiredFormat;
            }
        }

        // OutputF
        {
            unsigned int baseSize[3] = {
                m_output.texture_Output_size[0],
                m_output.texture_Output_size[1],
                m_output.texture_Output_size[2]
            };

            unsigned int desiredSize[3] = {
                ((baseSize[0] + 0) * 1) / 1 + 0,
                ((baseSize[1] + 0) * 1) / 1 + 0,
                ((baseSize[2] + 0) * 1) / 1 + 0
            };

            static const unsigned int desiredNumMips = 1;

            DXGI_FORMAT desiredFormat = DXGI_FORMAT_R32_FLOAT;

            if(!m_output.texture_OutputF ||
               m_output.texture_OutputF_size[0] != desiredSize[0] ||
               m_output.texture_OutputF_size[1] != desiredSize[1] ||
               m_output.texture_OutputF_size[2] != desiredSize[2] ||
               m_output.texture_OutputF_numMips != desiredNumMips ||
               m_output.texture_OutputF_format != desiredFormat)
            {
                dirty = true;
                if(m_output.texture_OutputF)
                    s_delayedRelease.Add(m_output.texture_OutputF);

                m_output.texture_OutputF = DX12Utils::CreateTexture(device, desiredSize, desiredNumMips, desiredFormat, m_output.texture_OutputF_flags, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, DX12Utils::ResourceType::Texture2D, (c_debugNames ? L"OutputF" : nullptr), Context::LogFn);
                m_output.texture_OutputF_size[0] = desiredSize[0];
                m_output.texture_OutputF_size[1] = desiredSize[1];
                m_output.texture_OutputF_size[2] = desiredSize[2];
                m_output.texture_OutputF_numMips = desiredNumMips;
                m_output.texture_OutputF_format = desiredFormat;
            }
        }

        // CombinedOutput
        {
            unsigned int baseSize[3] = { 1, 1, 1 };

            unsigned int desiredSize[3] = {
                ((baseSize[0] + 0) * 256) / 1 + 0,
                ((baseSize[1] + 0) * 256) / 1 + 0,
                ((baseSize[2] + 0) * 1) / 1 + 0
            };

            static const unsigned int desiredNumMips = 1;

            DXGI_FORMAT desiredFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

            if(!m_output.texture_CombinedOutput ||
               m_output.texture_CombinedOutput_size[0] != desiredSize[0] ||
               m_output.texture_CombinedOutput_size[1] != desiredSize[1] ||
               m_output.texture_CombinedOutput_size[2] != desiredSize[2] ||
               m_output.texture_CombinedOutput_numMips != desiredNumMips ||
               m_output.texture_CombinedOutput_format != desiredFormat)
            {
                dirty = true;
                if(m_output.texture_CombinedOutput)
                    s_delayedRelease.Add(m_output.texture_CombinedOutput);

                m_output.texture_CombinedOutput = DX12Utils::CreateTexture(device, desiredSize, desiredNumMips, desiredFormat, m_output.texture_CombinedOutput_flags, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, DX12Utils::ResourceType::Texture2D, (c_debugNames ? L"CombinedOutput" : nullptr), Context::LogFn);
                m_output.texture_CombinedOutput_size[0] = desiredSize[0];
                m_output.texture_CombinedOutput_size[1] = desiredSize[1];
                m_output.texture_CombinedOutput_size[2] = desiredSize[2];
                m_output.texture_CombinedOutput_numMips = desiredNumMips;
                m_output.texture_CombinedOutput_format = desiredFormat;
            }
        }

        // Histogram_Counts
        {

            unsigned int baseCount = (unsigned int)m_input.variable_Histogram_NumBuckets;
            unsigned int desiredCount = ((baseCount + 0 ) * 1) / 1 + 1;
            DXGI_FORMAT desiredFormat = DXGI_FORMAT_R32_UINT;
            unsigned int desiredStride = 0;

            if(!m_output.buffer_Histogram_Counts ||
               m_output.buffer_Histogram_Counts_count != desiredCount ||
               m_output.buffer_Histogram_Counts_format != desiredFormat ||
               m_output.buffer_Histogram_Counts_stride != desiredStride)
            {
                dirty = true;
                if(m_output.buffer_Histogram_Counts)
                    s_delayedRelease.Add(m_output.buffer_Histogram_Counts);

                unsigned int desiredSize = desiredCount * ((desiredStride > 0) ? desiredStride : DX12Utils::Get_DXGI_FORMAT_Info(desiredFormat, Context::LogFn).bytesPerPixel);

                m_output.buffer_Histogram_Counts = DX12Utils::CreateBuffer(device, desiredSize, m_output.c_buffer_Histogram_Counts_flags, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"Histogram_Counts" : nullptr), Context::LogFn);
                m_output.buffer_Histogram_Counts_count = desiredCount;
                m_output.buffer_Histogram_Counts_format = desiredFormat;
                m_output.buffer_Histogram_Counts_stride = desiredStride;
            }
        }

        // Histogram_Graph
        {

            unsigned int baseSize[3] = { (unsigned int)m_input.variable_Histogram_GraphSize[0], (unsigned int)m_input.variable_Histogram_GraphSize[1], 1 };

            unsigned int desiredSize[3] = {
                ((baseSize[0] + 0) * 1) / 1 + 0,
                ((baseSize[1] + 0) * 1) / 1 + 0,
                ((baseSize[2] + 0) * 1) / 1 + 0
            };

            static const unsigned int desiredNumMips = 1;

            DXGI_FORMAT desiredFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

            if(!m_output.texture_Histogram_Graph ||
               m_output.texture_Histogram_Graph_size[0] != desiredSize[0] ||
               m_output.texture_Histogram_Graph_size[1] != desiredSize[1] ||
               m_output.texture_Histogram_Graph_size[2] != desiredSize[2] ||
               m_output.texture_Histogram_Graph_numMips != desiredNumMips ||
               m_output.texture_Histogram_Graph_format != desiredFormat)
            {
                dirty = true;
                if(m_output.texture_Histogram_Graph)
                    s_delayedRelease.Add(m_output.texture_Histogram_Graph);

                m_output.texture_Histogram_Graph = DX12Utils::CreateTexture(device, desiredSize, desiredNumMips, desiredFormat, m_output.texture_Histogram_Graph_flags, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, DX12Utils::ResourceType::Texture2D, (c_debugNames ? L"Histogram_Graph" : nullptr), Context::LogFn);
                m_output.texture_Histogram_Graph_size[0] = desiredSize[0];
                m_output.texture_Histogram_Graph_size[1] = desiredSize[1];
                m_output.texture_Histogram_Graph_size[2] = desiredSize[2];
                m_output.texture_Histogram_Graph_numMips = desiredNumMips;
                m_output.texture_Histogram_Graph_format = desiredFormat;
            }
        }

        // Histogram_MinMaxValue
        {
            unsigned int baseCount = 1;
            unsigned int desiredCount = ((baseCount + 0 ) * 1) / 1 + 0;
            DXGI_FORMAT desiredFormat = DXGI_FORMAT_UNKNOWN;
            unsigned int desiredStride = 8;

            if(!m_output.buffer_Histogram_MinMaxValue ||
               m_output.buffer_Histogram_MinMaxValue_count != desiredCount ||
               m_output.buffer_Histogram_MinMaxValue_format != desiredFormat ||
               m_output.buffer_Histogram_MinMaxValue_stride != desiredStride)
            {
                dirty = true;
                if(m_output.buffer_Histogram_MinMaxValue)
                    s_delayedRelease.Add(m_output.buffer_Histogram_MinMaxValue);

                unsigned int desiredSize = desiredCount * ((desiredStride > 0) ? desiredStride : DX12Utils::Get_DXGI_FORMAT_Info(desiredFormat, Context::LogFn).bytesPerPixel);

                m_output.buffer_Histogram_MinMaxValue = DX12Utils::CreateBuffer(device, desiredSize, m_output.c_buffer_Histogram_MinMaxValue_flags, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"Histogram_MinMaxValue" : nullptr), Context::LogFn);
                m_output.buffer_Histogram_MinMaxValue_count = desiredCount;
                m_output.buffer_Histogram_MinMaxValue_format = desiredFormat;
                m_output.buffer_Histogram_MinMaxValue_stride = desiredStride;
            }
        }

        // DFT_DFTMagnitude
        {
            unsigned int baseSize[3] = {
                m_output.texture_OutputF_size[0],
                m_output.texture_OutputF_size[1],
                m_output.texture_OutputF_size[2]
            };

            unsigned int desiredSize[3] = {
                ((baseSize[0] + 0) * 1) / 1 + 0,
                ((baseSize[1] + 0) * 1) / 1 + 0,
                ((baseSize[2] + 0) * 1) / 1 + 0
            };

            static const unsigned int desiredNumMips = 1;

            DXGI_FORMAT desiredFormat = DXGI_FORMAT_R32_FLOAT;

            if(!m_output.texture_DFT_DFTMagnitude ||
               m_output.texture_DFT_DFTMagnitude_size[0] != desiredSize[0] ||
               m_output.texture_DFT_DFTMagnitude_size[1] != desiredSize[1] ||
               m_output.texture_DFT_DFTMagnitude_size[2] != desiredSize[2] ||
               m_output.texture_DFT_DFTMagnitude_numMips != desiredNumMips ||
               m_output.texture_DFT_DFTMagnitude_format != desiredFormat)
            {
                dirty = true;
                if(m_output.texture_DFT_DFTMagnitude)
                    s_delayedRelease.Add(m_output.texture_DFT_DFTMagnitude);

                m_output.texture_DFT_DFTMagnitude = DX12Utils::CreateTexture(device, desiredSize, desiredNumMips, desiredFormat, m_output.texture_DFT_DFTMagnitude_flags, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, DX12Utils::ResourceType::Texture2D, (c_debugNames ? L"DFT_DFTMagnitude" : nullptr), Context::LogFn);
                m_output.texture_DFT_DFTMagnitude_size[0] = desiredSize[0];
                m_output.texture_DFT_DFTMagnitude_size[1] = desiredSize[1];
                m_output.texture_DFT_DFTMagnitude_size[2] = desiredSize[2];
                m_output.texture_DFT_DFTMagnitude_numMips = desiredNumMips;
                m_output.texture_DFT_DFTMagnitude_format = desiredFormat;
            }
        }

        // DFT_MaxMagnitude
        {
            unsigned int baseCount = 1;
            unsigned int desiredCount = ((baseCount + 0 ) * 1) / 1 + 0;
            DXGI_FORMAT desiredFormat = DXGI_FORMAT_R32_UINT;
            unsigned int desiredStride = 0;

            if(!m_internal.buffer_DFT_MaxMagnitude ||
               m_internal.buffer_DFT_MaxMagnitude_count != desiredCount ||
               m_internal.buffer_DFT_MaxMagnitude_format != desiredFormat ||
               m_internal.buffer_DFT_MaxMagnitude_stride != desiredStride)
            {
                dirty = true;
                if(m_internal.buffer_DFT_MaxMagnitude)
                    s_delayedRelease.Add(m_internal.buffer_DFT_MaxMagnitude);

                unsigned int desiredSize = desiredCount * ((desiredStride > 0) ? desiredStride : DX12Utils::Get_DXGI_FORMAT_Info(desiredFormat, Context::LogFn).bytesPerPixel);

                m_internal.buffer_DFT_MaxMagnitude = DX12Utils::CreateBuffer(device, desiredSize, m_internal.c_buffer_DFT_MaxMagnitude_flags, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"DFT_MaxMagnitude" : nullptr), Context::LogFn);
                m_internal.buffer_DFT_MaxMagnitude_count = desiredCount;
                m_internal.buffer_DFT_MaxMagnitude_format = desiredFormat;
                m_internal.buffer_DFT_MaxMagnitude_stride = desiredStride;
            }
        }

        // _loadedTexture_0
        {
            if (!m_internal.texture__loadedTexture_0)
            {
                // Load the texture
                std::vector<DX12Utils::TextureCache::Texture> loadedTextureSlices;
                DX12Utils::DXGI_FORMAT_Info formatInfo = DX12Utils::Get_DXGI_FORMAT_Info(DXGI_FORMAT_R8_UNORM, Context::LogFn);
                DX12Utils::TextureCache::Type desiredType = DX12Utils::TextureCache::Type::U8;
                if (formatInfo.channelType == DX12Utils::DXGI_FORMAT_Info::ChannelType::_uint8_t)
                    desiredType = DX12Utils::TextureCache::Type::U8;
                else if (formatInfo.channelType == DX12Utils::DXGI_FORMAT_Info::ChannelType::_float)
                    desiredType = DX12Utils::TextureCache::Type::F32;
                else
                    Context::LogFn(LogLevel::Error, "Unhandled channel type");

                int textureIndex = -1;
                while(1)
                {
                    textureIndex++;
                    char indexedFileName[1024];
                    sprintf_s(indexedFileName, "%lsassets/Assets/real_uniform_gauss1_0_Gauss10_separate05_%i.png", s_techniqueLocation.c_str(), textureIndex);
                    DX12Utils::TextureCache::Texture loadedTextureSlice = DX12Utils::TextureCache::GetAs(indexedFileName, false, desiredType, formatInfo.sRGB, formatInfo.channelCount);

                    if(!loadedTextureSlice.Valid())
                    {
                        if (textureIndex == 0)
                            Context::LogFn(LogLevel::Error, "Could not load image: %s", indexedFileName);
                        break;
                    }

                    if (textureIndex > 0 && (loadedTextureSlice.width != loadedTextureSlices[0].width || loadedTextureSlice.height != loadedTextureSlices[0].height))
                        Context::LogFn(LogLevel::Error, "%s does not match dimensions of the first texture loaded!", indexedFileName);

                    loadedTextureSlices.push_back(loadedTextureSlice);
                }

                unsigned int size[3] = { (unsigned int)loadedTextureSlices[0].width, (unsigned int)loadedTextureSlices[0].height, (unsigned int)loadedTextureSlices.size() };

                static const unsigned int desiredNumMips = 1;

                // Create the texture
                dirty = true;
                m_internal.texture__loadedTexture_0_size[0] = size[0];
                m_internal.texture__loadedTexture_0_size[1] = size[1];
                m_internal.texture__loadedTexture_0_size[2] = size[2];
                m_internal.texture__loadedTexture_0_numMips = desiredNumMips;
                m_internal.texture__loadedTexture_0_format = DXGI_FORMAT_R8_UNORM;
                m_internal.texture__loadedTexture_0 = DX12Utils::CreateTexture(device, size, desiredNumMips, DXGI_FORMAT_R8_UNORM, m_internal.texture__loadedTexture_0_flags, D3D12_RESOURCE_STATE_COPY_DEST, DX12Utils::ResourceType::Texture2DArray, (c_debugNames ? L"_loadedTexture_0" : nullptr), Context::LogFn);


                std::vector<unsigned char> pixels;
                for (const DX12Utils::TextureCache::Texture& texture : loadedTextureSlices)
                    pixels.insert(pixels.end(), texture.pixels.begin(), texture.pixels.end());

                DX12Utils::UploadTextureToGPUAndMakeMips(device, commandList, s_ubTracker, m_internal.texture__loadedTexture_0, pixels, size, desiredNumMips, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, LogFn);
            }
        }

        // _loadedTexture_1
        {
            if (!m_internal.texture__loadedTexture_1)
            {
                // Load the texture
                std::vector<DX12Utils::TextureCache::Texture> loadedTextureSlices;
                DX12Utils::DXGI_FORMAT_Info formatInfo = DX12Utils::Get_DXGI_FORMAT_Info(DXGI_FORMAT_R8_UNORM, Context::LogFn);
                DX12Utils::TextureCache::Type desiredType = DX12Utils::TextureCache::Type::U8;
                if (formatInfo.channelType == DX12Utils::DXGI_FORMAT_Info::ChannelType::_uint8_t)
                    desiredType = DX12Utils::TextureCache::Type::U8;
                else if (formatInfo.channelType == DX12Utils::DXGI_FORMAT_Info::ChannelType::_float)
                    desiredType = DX12Utils::TextureCache::Type::F32;
                else
                    Context::LogFn(LogLevel::Error, "Unhandled channel type");

                int textureIndex = -1;
                while(1)
                {
                    textureIndex++;
                    char indexedFileName[1024];
                    sprintf_s(indexedFileName, "%lsassets/Assets/real_uniform_binomial3x3_Gauss10_separate05_%i.png", s_techniqueLocation.c_str(), textureIndex);
                    DX12Utils::TextureCache::Texture loadedTextureSlice = DX12Utils::TextureCache::GetAs(indexedFileName, false, desiredType, formatInfo.sRGB, formatInfo.channelCount);

                    if(!loadedTextureSlice.Valid())
                    {
                        if (textureIndex == 0)
                            Context::LogFn(LogLevel::Error, "Could not load image: %s", indexedFileName);
                        break;
                    }

                    if (textureIndex > 0 && (loadedTextureSlice.width != loadedTextureSlices[0].width || loadedTextureSlice.height != loadedTextureSlices[0].height))
                        Context::LogFn(LogLevel::Error, "%s does not match dimensions of the first texture loaded!", indexedFileName);

                    loadedTextureSlices.push_back(loadedTextureSlice);
                }

                unsigned int size[3] = { (unsigned int)loadedTextureSlices[0].width, (unsigned int)loadedTextureSlices[0].height, (unsigned int)loadedTextureSlices.size() };

                static const unsigned int desiredNumMips = 1;

                // Create the texture
                dirty = true;
                m_internal.texture__loadedTexture_1_size[0] = size[0];
                m_internal.texture__loadedTexture_1_size[1] = size[1];
                m_internal.texture__loadedTexture_1_size[2] = size[2];
                m_internal.texture__loadedTexture_1_numMips = desiredNumMips;
                m_internal.texture__loadedTexture_1_format = DXGI_FORMAT_R8_UNORM;
                m_internal.texture__loadedTexture_1 = DX12Utils::CreateTexture(device, size, desiredNumMips, DXGI_FORMAT_R8_UNORM, m_internal.texture__loadedTexture_1_flags, D3D12_RESOURCE_STATE_COPY_DEST, DX12Utils::ResourceType::Texture2DArray, (c_debugNames ? L"_loadedTexture_1" : nullptr), Context::LogFn);


                std::vector<unsigned char> pixels;
                for (const DX12Utils::TextureCache::Texture& texture : loadedTextureSlices)
                    pixels.insert(pixels.end(), texture.pixels.begin(), texture.pixels.end());

                DX12Utils::UploadTextureToGPUAndMakeMips(device, commandList, s_ubTracker, m_internal.texture__loadedTexture_1, pixels, size, desiredNumMips, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, LogFn);
            }
        }

        // _loadedTexture_2
        {
            if (!m_internal.texture__loadedTexture_2)
            {
                // Load the texture
                std::vector<DX12Utils::TextureCache::Texture> loadedTextureSlices;
                DX12Utils::DXGI_FORMAT_Info formatInfo = DX12Utils::Get_DXGI_FORMAT_Info(DXGI_FORMAT_R8_UNORM, Context::LogFn);
                DX12Utils::TextureCache::Type desiredType = DX12Utils::TextureCache::Type::U8;
                if (formatInfo.channelType == DX12Utils::DXGI_FORMAT_Info::ChannelType::_uint8_t)
                    desiredType = DX12Utils::TextureCache::Type::U8;
                else if (formatInfo.channelType == DX12Utils::DXGI_FORMAT_Info::ChannelType::_float)
                    desiredType = DX12Utils::TextureCache::Type::F32;
                else
                    Context::LogFn(LogLevel::Error, "Unhandled channel type");

                int textureIndex = -1;
                while(1)
                {
                    textureIndex++;
                    char indexedFileName[1024];
                    sprintf_s(indexedFileName, "%lsassets/Assets/real_uniform_box3x3_Gauss10_separate05_%i.png", s_techniqueLocation.c_str(), textureIndex);
                    DX12Utils::TextureCache::Texture loadedTextureSlice = DX12Utils::TextureCache::GetAs(indexedFileName, false, desiredType, formatInfo.sRGB, formatInfo.channelCount);

                    if(!loadedTextureSlice.Valid())
                    {
                        if (textureIndex == 0)
                            Context::LogFn(LogLevel::Error, "Could not load image: %s", indexedFileName);
                        break;
                    }

                    if (textureIndex > 0 && (loadedTextureSlice.width != loadedTextureSlices[0].width || loadedTextureSlice.height != loadedTextureSlices[0].height))
                        Context::LogFn(LogLevel::Error, "%s does not match dimensions of the first texture loaded!", indexedFileName);

                    loadedTextureSlices.push_back(loadedTextureSlice);
                }

                unsigned int size[3] = { (unsigned int)loadedTextureSlices[0].width, (unsigned int)loadedTextureSlices[0].height, (unsigned int)loadedTextureSlices.size() };

                static const unsigned int desiredNumMips = 1;

                // Create the texture
                dirty = true;
                m_internal.texture__loadedTexture_2_size[0] = size[0];
                m_internal.texture__loadedTexture_2_size[1] = size[1];
                m_internal.texture__loadedTexture_2_size[2] = size[2];
                m_internal.texture__loadedTexture_2_numMips = desiredNumMips;
                m_internal.texture__loadedTexture_2_format = DXGI_FORMAT_R8_UNORM;
                m_internal.texture__loadedTexture_2 = DX12Utils::CreateTexture(device, size, desiredNumMips, DXGI_FORMAT_R8_UNORM, m_internal.texture__loadedTexture_2_flags, D3D12_RESOURCE_STATE_COPY_DEST, DX12Utils::ResourceType::Texture2DArray, (c_debugNames ? L"_loadedTexture_2" : nullptr), Context::LogFn);


                std::vector<unsigned char> pixels;
                for (const DX12Utils::TextureCache::Texture& texture : loadedTextureSlices)
                    pixels.insert(pixels.end(), texture.pixels.begin(), texture.pixels.end());

                DX12Utils::UploadTextureToGPUAndMakeMips(device, commandList, s_ubTracker, m_internal.texture__loadedTexture_2, pixels, size, desiredNumMips, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, LogFn);
            }
        }

        // _loadedTexture_3
        {
            if (!m_internal.texture__loadedTexture_3)
            {
                // Load the texture
                std::vector<DX12Utils::TextureCache::Texture> loadedTextureSlices;
                DX12Utils::DXGI_FORMAT_Info formatInfo = DX12Utils::Get_DXGI_FORMAT_Info(DXGI_FORMAT_R8_UNORM, Context::LogFn);
                DX12Utils::TextureCache::Type desiredType = DX12Utils::TextureCache::Type::U8;
                if (formatInfo.channelType == DX12Utils::DXGI_FORMAT_Info::ChannelType::_uint8_t)
                    desiredType = DX12Utils::TextureCache::Type::U8;
                else if (formatInfo.channelType == DX12Utils::DXGI_FORMAT_Info::ChannelType::_float)
                    desiredType = DX12Utils::TextureCache::Type::F32;
                else
                    Context::LogFn(LogLevel::Error, "Unhandled channel type");

                int textureIndex = -1;
                while(1)
                {
                    textureIndex++;
                    char indexedFileName[1024];
                    sprintf_s(indexedFileName, "%lsassets/Assets/real_uniform_box5x5_Gauss10_separate05_%i.png", s_techniqueLocation.c_str(), textureIndex);
                    DX12Utils::TextureCache::Texture loadedTextureSlice = DX12Utils::TextureCache::GetAs(indexedFileName, false, desiredType, formatInfo.sRGB, formatInfo.channelCount);

                    if(!loadedTextureSlice.Valid())
                    {
                        if (textureIndex == 0)
                            Context::LogFn(LogLevel::Error, "Could not load image: %s", indexedFileName);
                        break;
                    }

                    if (textureIndex > 0 && (loadedTextureSlice.width != loadedTextureSlices[0].width || loadedTextureSlice.height != loadedTextureSlices[0].height))
                        Context::LogFn(LogLevel::Error, "%s does not match dimensions of the first texture loaded!", indexedFileName);

                    loadedTextureSlices.push_back(loadedTextureSlice);
                }

                unsigned int size[3] = { (unsigned int)loadedTextureSlices[0].width, (unsigned int)loadedTextureSlices[0].height, (unsigned int)loadedTextureSlices.size() };

                static const unsigned int desiredNumMips = 1;

                // Create the texture
                dirty = true;
                m_internal.texture__loadedTexture_3_size[0] = size[0];
                m_internal.texture__loadedTexture_3_size[1] = size[1];
                m_internal.texture__loadedTexture_3_size[2] = size[2];
                m_internal.texture__loadedTexture_3_numMips = desiredNumMips;
                m_internal.texture__loadedTexture_3_format = DXGI_FORMAT_R8_UNORM;
                m_internal.texture__loadedTexture_3 = DX12Utils::CreateTexture(device, size, desiredNumMips, DXGI_FORMAT_R8_UNORM, m_internal.texture__loadedTexture_3_flags, D3D12_RESOURCE_STATE_COPY_DEST, DX12Utils::ResourceType::Texture2DArray, (c_debugNames ? L"_loadedTexture_3" : nullptr), Context::LogFn);


                std::vector<unsigned char> pixels;
                for (const DX12Utils::TextureCache::Texture& texture : loadedTextureSlices)
                    pixels.insert(pixels.end(), texture.pixels.begin(), texture.pixels.end());

                DX12Utils::UploadTextureToGPUAndMakeMips(device, commandList, s_ubTracker, m_internal.texture__loadedTexture_3, pixels, size, desiredNumMips, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, LogFn);
            }
        }

        // _DisplayCSCB
        if (m_internal.constantBuffer__DisplayCSCB == nullptr)
        {
            dirty = true;
            m_internal.constantBuffer__DisplayCSCB = DX12Utils::CreateBuffer(device, 256, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"_DisplayCSCB" : nullptr), Context::LogFn);
        }

        // _Histogram_MakeCountsCSCB
        if (m_internal.constantBuffer__Histogram_MakeCountsCSCB == nullptr)
        {
            dirty = true;
            m_internal.constantBuffer__Histogram_MakeCountsCSCB = DX12Utils::CreateBuffer(device, 256, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"_Histogram_MakeCountsCSCB" : nullptr), Context::LogFn);
        }

        // _Histogram_MakeGraphCSCB
        if (m_internal.constantBuffer__Histogram_MakeGraphCSCB == nullptr)
        {
            dirty = true;
            m_internal.constantBuffer__Histogram_MakeGraphCSCB = DX12Utils::CreateBuffer(device, 256, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"_Histogram_MakeGraphCSCB" : nullptr), Context::LogFn);
        }

        // _Histogram_MakeMinMaxValueCSCB
        if (m_internal.constantBuffer__Histogram_MakeMinMaxValueCSCB == nullptr)
        {
            dirty = true;
            m_internal.constantBuffer__Histogram_MakeMinMaxValueCSCB = DX12Utils::CreateBuffer(device, 256, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"_Histogram_MakeMinMaxValueCSCB" : nullptr), Context::LogFn);
        }

        // _DFT_DFTCSCB
        if (m_internal.constantBuffer__DFT_DFTCSCB == nullptr)
        {
            dirty = true;
            m_internal.constantBuffer__DFT_DFTCSCB = DX12Utils::CreateBuffer(device, 256, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"_DFT_DFTCSCB" : nullptr), Context::LogFn);
        }

        // _DFT_NormalizeCSCB
        if (m_internal.constantBuffer__DFT_NormalizeCSCB == nullptr)
        {
            dirty = true;
            m_internal.constantBuffer__DFT_NormalizeCSCB = DX12Utils::CreateBuffer(device, 256, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (c_debugNames ? L"_DFT_NormalizeCSCB" : nullptr), Context::LogFn);
        }
        EnsureDrawCallPSOsCreated(device, dirty);
    }

    bool Context::EnsureDrawCallPSOsCreated(ID3D12Device* device, bool dirty)
    {
        return true;
    }
};
