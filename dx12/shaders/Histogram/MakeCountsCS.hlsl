// Unnamed technique, shader MakeCountsCS


struct Struct__Histogram_MakeCountsCSCB
{
    uint Histogram_NumBuckets;
    uint Histogram_ZeroMinMaxBucket;
    float2 _padding0;
};

Texture2D<float4> Input : register(t0);
StructuredBuffer<uint2> MinMaxValue : register(t1);
RWStructuredBuffer<uint> Counts : register(u0);
ConstantBuffer<Struct__Histogram_MakeCountsCSCB> _Histogram_MakeCountsCSCB : register(b0);

#line 2


[numthreads(8, 8, 1)]
#line 4
void csmain(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;

	float value = dot(Input[px].xyzw, float4(1.0f, 0.0f, 0.0f, 0.0f));
	float valuePercent = (value - asfloat(MinMaxValue[0].x)) / (asfloat(MinMaxValue[0].y) - asfloat(MinMaxValue[0].x));

	uint bucketIndex = uint(valuePercent * float(_Histogram_MakeCountsCSCB.Histogram_NumBuckets-1));

	// don't count min and max buckets if we aren't supposed to
	if (_Histogram_MakeCountsCSCB.Histogram_ZeroMinMaxBucket && (bucketIndex == 0 || bucketIndex == (_Histogram_MakeCountsCSCB.Histogram_NumBuckets-1)))
		return;

	uint oldValue;
	InterlockedAdd(Counts[bucketIndex+1], 1, oldValue);

	uint oldMax;
	InterlockedMax(Counts[0], oldValue+1, oldMax);
}

/*
Shader Resources:
	Texture Input (as SRV)
	Buffer MinVaxValue (as SRV)
	Buffer Counts (as UAV)
*/
