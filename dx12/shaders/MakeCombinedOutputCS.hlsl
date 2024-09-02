// Unnamed technique, shader MakeCombinedOutputCS


struct NoiseTypes
{
    static const int Blue = 0;
    static const int White = 1;
    static const int Binomial3x3 = 2;
    static const int Box3x3 = 3;
    static const int Box5x5 = 4;
    static const int Perlin = 5;
    static const int R2 = 6;
    static const int IGN = 7;
};

RWTexture2D<float4> CombinedOutput : register(u0);
Texture2D<float4> Noise : register(t0);
Texture2D<float4> Histogram : register(t1);
Texture2D<float4> DFT : register(t2);

#line 2


[numthreads(8, 8, 1)]
#line 4
void csmain(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;
	if (px.y >= 128)
	{
		uint2 srcpx = px - uint2(0, 128);
		CombinedOutput[px] = float4(Histogram[srcpx].rrr, 1.0f);
		return;
	}

	if (px.x >= 128)
	{
		uint2 srcpx = px - uint2(128, 0);
		CombinedOutput[px] = float4(DFT[srcpx].rrr, 1.0f);
		return;
	}

	CombinedOutput[px] = float4(Noise[px].rrr, 1.0f);
}

/*
Shader Resources:
	Texture CombinedOutput (as UAV)
	Texture Noise (as SRV)
	Texture DFT (as SRV)
	Texture Histogram (as SRV)
*/
