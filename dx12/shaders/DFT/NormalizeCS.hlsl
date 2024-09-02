// Unnamed technique, shader NormalizeCS


struct Struct__DFT_NormalizeCSCB
{
    uint DFT_LogSpaceMagnitude;
    float3 _padding0;
};

RWTexture2D<float4> DFTMagnitude : register(u0);
StructuredBuffer<uint> MaxMagnitude : register(t0);
ConstantBuffer<Struct__DFT_NormalizeCSCB> _DFT_NormalizeCSCB : register(b0);

#line 2


[numthreads(8, 8, 1)]
#line 4
void csmain(uint3 DTid : SV_DispatchThreadID)
{
	float maxMagnitude = asfloat(MaxMagnitude[0]);
	uint2 px = DTid.xy;
	float value = DFTMagnitude[px];
	if (_DFT_NormalizeCSCB.DFT_LogSpaceMagnitude)
		DFTMagnitude[px] = log(1.0f + value * 255.0f);
	else
		DFTMagnitude[px] = value / maxMagnitude;
}

/*
Shader Resources:
	Texture DFTMagnitude (as UAV)
	Buffer MaxMagnitude (as SRV)
*/
