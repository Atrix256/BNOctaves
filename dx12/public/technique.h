#pragma once

#include "../private/technique.h"
#include <string>
#include <vector>
#include "DX12Utils/logfn.h"
#include "DX12Utils/dxutils.h"

namespace BNOctaves
{
    // Compile time technique settings. Feel free to modify these.
    static const int c_numSRVDescriptors = 256;  // If 0, no heap will be created. One heap shared by all contexts of this technique.
    static const int c_numRTVDescriptors = 256;  // If 0, no heap will be created. One heap shared by all contexts of this technique.
    static const int c_numDSVDescriptors = 256;  // If 0, no heap will be created. One heap shared by all contexts of this technique.
    static const bool c_debugShaders = true; // If true, will compile shaders with debug info enabled.
    static const bool c_debugNames = true; // If true, will set debug names on objects. If false, debug names should be deadstripped from the executable.

    // Information about the technique
    static const bool c_requiresRaytracing = false; // If true, this technique will not work without raytracing support

    using TPerfEventBeginFn = void (*)(const char* name, ID3D12GraphicsCommandList* commandList, int index);
    using TPerfEventEndFn = void (*)(ID3D12GraphicsCommandList* commandList);

    struct ProfileEntry
    {
        const char* m_label = nullptr;
        float m_gpu = 0.0f;
        float m_cpu = 0.0f;
    };

    struct Context
    {
        static const char* GetTechniqueName()
        {
            return "BNOctaves";
        }

        static const wchar_t* GetTechniqueNameW()
        {
            return L"BNOctaves";
        }

        // This is the input to the technique that you are expected to fill out
        struct ContextInput
        {

            // Variables
            NoiseTypes variable_NoiseType = NoiseTypes::Blue;  // The type of noise to use
            uint variable_NumberOfOctaves = 3;  // How many octaves to use
            bool variable_DifferentNoisePerOctave = false;  // If false, the same noise will be used for each octave. If true, a different noise, of the same type, will be used for each octave.
            uint variable_RNGSeed = 1337;  // A PRNG is used for various things, change this value to change thats eed.
            uint variable_PerlinCellSize = 128;
            float2 variable_PerlinMinMax = {0.0f, 1.0f};  // Perlin noise can go below zero which causes problems in this demo. To help that, this is the range of values which are mapped to [0,1]. Anything lower than 0 is clipped to 0 after the remapping.
            uint variable_Histogram_NumBuckets = 256;
            uint2 variable_Histogram_GraphSize = {256, 128};
            float2 variable_Histogram_XAxisRange = {0.0f, 1.0f};
            bool variable_Histogram_AutoXAxisRange = true;
            bool variable_Histogram_ZeroMinMaxBucket = false;  // If values are clamped to a min and max value, the min and max bucket will have too many counts in them. This option zeros them out to make the rest of the data easier to see.
            bool variable_DFT_RemoveDC = true;  // DC (0hz) is often a large spike that makes it hard to see the rest of the frequencies. Use this to set DC to zero.
            bool variable_DFT_LogSpaceMagnitude = true;  // If true, show magnitude in log space
        };
        ContextInput m_input;

        // This is the output of the technique that you can consume
        struct ContextOutput
        {

            ID3D12Resource* texture_Output = nullptr;
            unsigned int texture_Output_size[3] = { 0, 0, 0 };
            unsigned int texture_Output_numMips = 0;
            DXGI_FORMAT texture_Output_format = DXGI_FORMAT_UNKNOWN;
            static const D3D12_RESOURCE_FLAGS texture_Output_flags =  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            const D3D12_RESOURCE_STATES c_texture_Output_endingState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

            ID3D12Resource* texture_OutputF = nullptr;
            unsigned int texture_OutputF_size[3] = { 0, 0, 0 };
            unsigned int texture_OutputF_numMips = 0;
            DXGI_FORMAT texture_OutputF_format = DXGI_FORMAT_UNKNOWN;
            static const D3D12_RESOURCE_FLAGS texture_OutputF_flags =  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            const D3D12_RESOURCE_STATES c_texture_OutputF_endingState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

            ID3D12Resource* texture_CombinedOutput = nullptr;
            unsigned int texture_CombinedOutput_size[3] = { 0, 0, 0 };
            unsigned int texture_CombinedOutput_numMips = 0;
            DXGI_FORMAT texture_CombinedOutput_format = DXGI_FORMAT_UNKNOWN;
            static const D3D12_RESOURCE_FLAGS texture_CombinedOutput_flags =  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            const D3D12_RESOURCE_STATES c_texture_CombinedOutput_endingState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

            ID3D12Resource* buffer_Histogram_Counts = nullptr;
            DXGI_FORMAT buffer_Histogram_Counts_format = DXGI_FORMAT_UNKNOWN; // For typed buffers, the type of the buffer
            unsigned int buffer_Histogram_Counts_stride = 0; // For structured buffers, the size of the structure
            unsigned int buffer_Histogram_Counts_count = 0; // How many items there are
            const D3D12_RESOURCE_STATES c_buffer_Histogram_Counts_endingState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

            static const D3D12_RESOURCE_FLAGS c_buffer_Histogram_Counts_flags =  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS; // Flags the buffer needs to have been created with

            ID3D12Resource* texture_Histogram_Graph = nullptr;
            unsigned int texture_Histogram_Graph_size[3] = { 0, 0, 0 };
            unsigned int texture_Histogram_Graph_numMips = 0;
            DXGI_FORMAT texture_Histogram_Graph_format = DXGI_FORMAT_UNKNOWN;
            static const D3D12_RESOURCE_FLAGS texture_Histogram_Graph_flags =  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            const D3D12_RESOURCE_STATES c_texture_Histogram_Graph_endingState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

            ID3D12Resource* buffer_Histogram_MinMaxValue = nullptr;
            DXGI_FORMAT buffer_Histogram_MinMaxValue_format = DXGI_FORMAT_UNKNOWN; // For typed buffers, the type of the buffer
            unsigned int buffer_Histogram_MinMaxValue_stride = 0; // For structured buffers, the size of the structure
            unsigned int buffer_Histogram_MinMaxValue_count = 0; // How many items there are
            const D3D12_RESOURCE_STATES c_buffer_Histogram_MinMaxValue_endingState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

            static const D3D12_RESOURCE_FLAGS c_buffer_Histogram_MinMaxValue_flags =  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS; // Flags the buffer needs to have been created with

            ID3D12Resource* texture_DFT_DFTMagnitude = nullptr;
            unsigned int texture_DFT_DFTMagnitude_size[3] = { 0, 0, 0 };
            unsigned int texture_DFT_DFTMagnitude_numMips = 0;
            DXGI_FORMAT texture_DFT_DFTMagnitude_format = DXGI_FORMAT_UNKNOWN;
            static const D3D12_RESOURCE_FLAGS texture_DFT_DFTMagnitude_flags =  D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            const D3D12_RESOURCE_STATES c_texture_DFT_DFTMagnitude_endingState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        };
        ContextOutput m_output;

        // Internal storage for the technique
        ContextInternal m_internal;

        // If true, will do both cpu and gpu profiling. Call ReadbackProfileData() on the context to get the profiling data.
        bool m_profile = false;
        const ProfileEntry* ReadbackProfileData(ID3D12CommandQueue* commandQueue, int& numItems);

        // Set this static function pointer to your own log function if you want to recieve callbacks on info, warnings and errors.
        static TLogFn LogFn;

        // These callbacks are for perf instrumentation, such as with Pix.
        static TPerfEventBeginFn PerfEventBeginFn;
        static TPerfEventEndFn PerfEventEndFn;

        // The path to where the shader files for this technique are. Defaults to L"./"
        static std::wstring s_techniqueLocation;

        static int GetContextCount();
        static Context* GetContext(int index);

        // Buffer Creation
        template <typename T>
        ID3D12Resource* CreateManagedBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, const T* data, size_t count, const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState = D3D12_RESOURCE_STATE_COPY_DEST)
        {
            return CreateManagedBuffer(device, commandList, flags, (void*)data, count * sizeof(T), debugName, desiredState);
        }

        template <typename T>
        ID3D12Resource* CreateManagedBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, const T& data, const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState = D3D12_RESOURCE_STATE_COPY_DEST)
        {
            return CreateManagedBuffer(device, commandList, flags, (void*)&data, sizeof(T), debugName, desiredState);
        }

        template <typename T>
        ID3D12Resource* CreateManagedBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, const std::vector<T>& data, const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState = D3D12_RESOURCE_STATE_COPY_DEST)
        {
            return CreateManagedBuffer(device, commandList, flags, (void*)data.data(), data.size() * sizeof(T), debugName, desiredState);
        }

        ID3D12Resource* CreateManagedBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, const void* data, size_t size, const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState = D3D12_RESOURCE_STATE_COPY_DEST);

        // Texture Creation

        ID3D12Resource* CreateManagedTexture(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, DXGI_FORMAT format, const unsigned int size[3], unsigned int numMips, DX12Utils::ResourceType resourceType, const void* initialData, const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState = D3D12_RESOURCE_STATE_COPY_DEST);
        ID3D12Resource* CreateManagedTextureAndClear(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, DXGI_FORMAT format, const unsigned int size[3], unsigned int numMips, DX12Utils::ResourceType resourceType, void* clearValue, size_t clearValueSize, const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState = D3D12_RESOURCE_STATE_COPY_DEST);
        ID3D12Resource* CreateManagedTextureFromFile(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_FLAGS flags, DXGI_FORMAT format, DX12Utils::ResourceType resourceType, const char* fileName, bool sourceIsSRGB, unsigned int size[3], const wchar_t* debugName, D3D12_RESOURCE_STATES desiredState = D3D12_RESOURCE_STATE_COPY_DEST);

        // Helpers for the host app
        void UploadTextureData(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12Resource* texture, D3D12_RESOURCE_STATES textureState, const void* data, unsigned int unalignedPitch);
        void UploadBufferData(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12Resource* buffer, D3D12_RESOURCE_STATES bufferState, const void* data, unsigned int dataSize);

        // The resource will be freed when the context is destroyed
        void AddManagedResource(ID3D12Resource* resource)
        {
            m_internal.m_managedResources.push_back(resource);
        }

        // Returns the allocated index within the respective heap
        int GetRTV(ID3D12Device* device, ID3D12Resource* resource, DXGI_FORMAT format, D3D12_RTV_DIMENSION dimension, int arrayIndex, int mipIndex, const char* debugName);
        int GetDSV(ID3D12Device* device, ID3D12Resource* resource, DXGI_FORMAT format, D3D12_DSV_DIMENSION dimension, int arrayIndex, int mipIndex, const char* debugName);

        bool CreateManagedTLAS(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12Resource* vertexBuffer, int vertexBufferCount, bool isAABBs, D3D12_RAYTRACING_GEOMETRY_FLAGS geometryFlags, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, DXGI_FORMAT vertexPositionFormat, unsigned int vertexPositionOffset, unsigned int vertexPositionStride, ID3D12Resource*& blas, unsigned int& blasSize, ID3D12Resource*& tlas, unsigned int& tlasSize, TLogFn logFn)
        {
            ID3D12Resource* scratch = nullptr;
            ID3D12Resource* instanceDescs = nullptr;

            if (!DX12Utils::CreateTLAS(device, commandList, vertexBuffer, vertexBufferCount, isAABBs, geometryFlags, buildFlags, vertexPositionFormat, vertexPositionOffset, vertexPositionStride, blas, blasSize, tlas, tlasSize, scratch, instanceDescs, LogFn))
                return false;

            AddManagedResource(scratch);
            AddManagedResource(instanceDescs);

            AddManagedResource(blas);
            AddManagedResource(tlas);

            return true;
        }

    private:
        friend void DestroyContext(Context* context);
        ~Context();

        friend void Execute(Context* context, ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
        void EnsureResourcesCreated(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
        bool EnsureDrawCallPSOsCreated(ID3D12Device* device, bool dirty);

        ProfileEntry m_profileData[9+1]; // One for each action node, and another for the total
    };

    struct ScopedPerfEvent
    {
        ScopedPerfEvent(const char* name, ID3D12GraphicsCommandList* commandList, int index)
            : m_commandList(commandList)
        {
            Context::PerfEventBeginFn(name, commandList, index);
        }

        ~ScopedPerfEvent()
        {
            Context::PerfEventEndFn(m_commandList);
        }

        ID3D12GraphicsCommandList* m_commandList;
    };

    // Create 0 to N contexts at any point
    Context* CreateContext(ID3D12Device* device);

    // Call at the beginning of your frame
    void OnNewFrame(int framesInFlight);

    // Call this 0 to M times a frame on each context to execute the technique
    void Execute(Context* context, ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

    // Destroy a context
    void DestroyContext(Context* context);
};
