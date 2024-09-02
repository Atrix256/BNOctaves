// Unnamed technique, shader MakeMinMaxValueCS


struct Struct__Histogram_MakeMinMaxValueCSCB
{
    uint Histogram_AutoXAxisRange;
    float2 Histogram_XAxisRange;
    float _padding0;
};

Texture2D<float4> Input : register(t0);
RWStructuredBuffer<uint2> MinMaxValue : register(u0);
ConstantBuffer<Struct__Histogram_MakeMinMaxValueCSCB> _Histogram_MakeMinMaxValueCSCB : register(b0);

#line 2


[numthreads(8, 8, 1)]
#line 4
void csmain(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;

	float value = dot(Input[px].xyzw, float4(1.0f, 0.0f, 0.0f, 0.0f));
	uint valueU = asuint(value);
	
	if (_Histogram_MakeMinMaxValueCSCB.Histogram_AutoXAxisRange)
	{
		uint dummy;
		InterlockedMin(MinMaxValue[0].x, valueU, dummy);
		InterlockedMax(MinMaxValue[0].y, valueU, dummy);
	}
	else
	{
		MinMaxValue[0] = uint2(asuint(_Histogram_MakeMinMaxValueCSCB.Histogram_XAxisRange.x), asuint(_Histogram_MakeMinMaxValueCSCB.Histogram_XAxisRange.y));
	}
}

/*
Shader Resources:
	Texture Input (as SRV)
	Texture MinMaxValue (as UAV)
*/
