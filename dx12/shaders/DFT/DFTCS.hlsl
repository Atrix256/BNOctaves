// Unnamed technique, shader DFTCS


struct Struct__DFT_DFTCSCB
{
    uint DFT_RemoveDC;
    float3 _padding0;
};

Texture2D<float4> Input : register(t0);
RWTexture2D<float2> Output : register(u0);
RWStructuredBuffer<uint> MaxMagnitude : register(u1);
ConstantBuffer<Struct__DFT_DFTCSCB> _DFT_DFTCSCB : register(b0);

#line 2


static const float c_pi = 3.14159265359f;

[numthreads(8, 8, 1)]
#line 6
void csmain(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;
	float K = float(px.x);
	float L = float(px.y);

	uint2 dims;
	Input.GetDimensions(dims.x, dims.y);

	float2 dft = float2(0.0f, 0.0f);

	for (uint y = 0; y < dims.y; y++)
	{
		float vy = L * float(y) / float(dims.y);
		for (uint x = 0; x < dims.x; x++)
		{
			float vx = K * float(x) / float(dims.x);

			float v = vx + vy;

			float theta = -2.0f * c_pi * v;

			float pixelValue = dot(Input[uint2(x,y)].xyzw, float4(1.0f, 0.0f, 0.0f, 0.0f));

			dft.x += cos(theta) * pixelValue;
			dft.y += sin(theta) * pixelValue;
		}
	}

	// 1/N in front of the DFT
	dft *= 1.0f / float(dims.x*dims.y);

	// Zero out DC (0hz) if we should
	if (_DFT_DFTCSCB.DFT_RemoveDC && px.x == 0 && px.y == 0)
		dft = float2(0.0f, 0.0f);

	// DFT shift
	px = (px + uint2(dims.x, dims.y) / 2) % dims;

	// Write out magnitude
	float magnitude = length(dft);
	Output[px] = magnitude;

	// keep track of maximum magnitude
	uint dummy;
	InterlockedMax(MaxMagnitude[0], asuint(magnitude), dummy);
}

/*
Shader Resources:
	Texture Input (as SRV)
	Texture Output (as UAV)
	Buffer MaxMagnitude (as UAV)
*/
