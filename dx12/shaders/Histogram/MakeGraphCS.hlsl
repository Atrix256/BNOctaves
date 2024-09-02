// Unnamed technique, shader MakeGraphCS


struct Struct__Histogram_MakeGraphCSCB
{
    uint2 Histogram_GraphSize;
    uint Histogram_NumBuckets;
    float _padding0;
};

StructuredBuffer<uint> Counts : register(t0);
RWTexture2D<float4> Output : register(u0);
ConstantBuffer<Struct__Histogram_MakeGraphCSCB> _Histogram_MakeGraphCSCB : register(b0);

#line 2


[numthreads(8, 8, 1)]
#line 4
void csmain(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;
	float countIndexF = (_Histogram_MakeGraphCSCB.Histogram_NumBuckets-1) * float(px.x) / float(_Histogram_MakeGraphCSCB.Histogram_GraphSize.x-1);
	uint countIndex = uint(countIndexF);
	uint count1 = Counts[min(countIndex+1, _Histogram_MakeGraphCSCB.Histogram_NumBuckets - 1)];
	uint count2 = Counts[min(countIndex+2, _Histogram_MakeGraphCSCB.Histogram_NumBuckets - 1)];
	float count = lerp(float(count1), float(count2), frac(countIndexF));
	float2 yRange = float2(0.0f, Counts[0] * 1.1f);
	float countPercent = (count - yRange.x) / (yRange.y - yRange.x);

	float2 uv = float2(px) / float2(_Histogram_MakeGraphCSCB.Histogram_GraphSize);
	uv.y = 1.0f - uv.y;

	float shade = (uv.y < countPercent) ? 0.0f : 1.0f;

	Output[px] = float4(shade.xxx, 1.0f);
}

/*
Shader Resources:
	Buffer Counts (as SRV)
	Texture Output (as UAV)
*/
