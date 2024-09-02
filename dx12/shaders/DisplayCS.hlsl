// Unnamed technique, shader DisplayCS


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

struct Struct__DisplayCSCB
{
    uint DifferentNoisePerOctave;
    int NoiseType;
    uint NumberOfOctaves;
    uint PerlinCellSize;
    float2 PerlinMinMax;
    uint RNGSeed;
    float _padding0;
};

RWTexture2D<float4> Output : register(u0);
RWTexture2D<float4> OutputF : register(u1);
Texture2DArray<float> _loadedTexture_0 : register(t0);
Texture2DArray<float> _loadedTexture_1 : register(t1);
Texture2DArray<float> _loadedTexture_2 : register(t2);
Texture2DArray<float> _loadedTexture_3 : register(t3);
ConstantBuffer<Struct__DisplayCSCB> _DisplayCSCB : register(b0);

#line 2


static const float c_pi = 3.14159265359f;

uint wang_hash_init(uint3 seed)
{
	return uint(seed.x * uint(1973) + seed.y * uint(9277) + seed.z * uint(26699)) | uint(1);
}

uint wang_hash_uint(inout uint seed)
{
	seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
	seed *= uint(9);
	seed = seed ^ (seed >> 4);
	seed *= uint(0x27d4eb2d);
	seed = seed ^ (seed >> 15);
	return seed;
}

float wang_hash_float01(inout uint state)
{
	return float(wang_hash_uint(state) & 0x00FFFFFF) / float(0x01000000);
}

// R2 Low discrepancy grid
// A generalization of the golden ratio to 2D
// From https://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/
float R2LDG(uint2 pos)
{
	static const float g  = 1.32471795724474602596f;
	static const float a1 = 1 / g;
	static const float a2 = 1 / (g * g);
	return frac(float(pos.x) * a1 + float(pos.y) * a2);
}

// Interleaved gradient noise
// From http://www.iryoku.com/next-generation-post-processing-in-call-of-duty-advanced-warfare
float IGNLDG(uint2 pos)
{
	return (52.9829189f * ((0.06711056f * float(pos.x) + 0.00583715f * float(pos.y)) % 1)) % 1;
}

// For perlin noise
float SmootherStep(float edge0, float edge1, float x)
{
    if (edge0 == edge1)
        return edge0;

    x = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);

    return 6.0f * x * x * x * x * x - 15.0f * x * x * x * x + 10.0f * x * x * x;
}

float2 PerlinNoise_UnitVectorAtCell(uint2 cellIndex, uint octave)
{
	uint rng = wang_hash_init(uint3(cellIndex, _DisplayCSCB.RNGSeed ^ (_DisplayCSCB.DifferentNoisePerOctave ? octave : 0)));
	float angle = wang_hash_float01(rng) * c_pi * 2.0f;
	return float2(cos(angle), sin(angle));
}

float PerlinNoise_DotGridGradient(uint2 cellPos, uint2 pos, uint cellSize, uint octave)
{
	float2 gradient = PerlinNoise_UnitVectorAtCell(cellPos / cellSize, octave);
	int2 delta = int2(pos) - int2(cellPos);
	float2 deltaF = float2(delta) / float(cellSize);
	return dot(deltaF, gradient);
}

// Perlin Noise
float PerlinNoise(uint2 pos, uint cellSize, uint octave)
{
	uint2 cellIndex = pos / cellSize;
	uint2 cellFractionU = pos - (cellIndex * cellSize);
	float2 cellFraction = float2(cellFractionU) / float(cellSize);

	// smoothstep the UVs
    cellFraction.x = SmootherStep(0.0f, 1.0f, cellFraction[0]);
    cellFraction.y = SmootherStep(0.0f, 1.0f, cellFraction[1]);	

    // get the 4 corners of the square
    float dg_00 = PerlinNoise_DotGridGradient((cellIndex + uint2( 0, 0 )) * cellSize, pos, cellSize, octave);
    float dg_01 = PerlinNoise_DotGridGradient((cellIndex + uint2( 0, 1 )) * cellSize, pos, cellSize, octave);
    float dg_10 = PerlinNoise_DotGridGradient((cellIndex + uint2( 1, 0 )) * cellSize, pos, cellSize, octave);
    float dg_11 = PerlinNoise_DotGridGradient((cellIndex + uint2( 1, 1 )) * cellSize, pos, cellSize, octave);

    // X interpolate
    float dg_x0 = lerp(dg_00, dg_10, cellFraction.x);
    float dg_x1 = lerp(dg_01, dg_11, cellFraction.x);

    // Y interpolate
    return lerp(dg_x0, dg_x1, cellFraction.y);
}

float SampleNoiseTexture(uint2 px, uint octave)
{
	switch(_DisplayCSCB.NoiseType)
	{
		case NoiseTypes::Blue:
		{
			uint3 dims;
			_loadedTexture_0.GetDimensions(dims.x, dims.y, dims.z);
			px = (px * (1U << octave)) % dims.xy;
			uint textureIndex = _DisplayCSCB.DifferentNoisePerOctave ? (octave % dims.z ) : 0;
			return _loadedTexture_0[uint3(px, textureIndex)];
		}		
		case NoiseTypes::White:
		{
			px = (px * (1U << octave));
			uint textureIndex = _DisplayCSCB.DifferentNoisePerOctave ? octave : 0;
			uint rng = wang_hash_init(uint3(px, _DisplayCSCB.RNGSeed ^ textureIndex));
			return wang_hash_float01(rng);
		}
		case NoiseTypes::Binomial3x3:
		{
			uint3 dims;
			_loadedTexture_1.GetDimensions(dims.x, dims.y, dims.z);
			px = (px * (1U << octave)) % dims.xy;
			uint textureIndex = _DisplayCSCB.DifferentNoisePerOctave ? (octave % dims.z ) : 0;
			return _loadedTexture_1[uint3(px, textureIndex)];
		}
		case NoiseTypes::Box3x3:
		{
			uint3 dims;
			_loadedTexture_2.GetDimensions(dims.x, dims.y, dims.z);
			px = (px * (1U << octave)) % dims.xy;
			uint textureIndex = _DisplayCSCB.DifferentNoisePerOctave ? (octave % dims.z ) : 0;
			return _loadedTexture_2[uint3(px, textureIndex)];
		}
		case NoiseTypes::Box5x5:
		{
			uint3 dims;
			_loadedTexture_3.GetDimensions(dims.x, dims.y, dims.z);
			px = (px * (1U << octave)) % dims.xy;
			uint textureIndex = _DisplayCSCB.DifferentNoisePerOctave ? (octave % dims.z ) : 0;
			return _loadedTexture_3[uint3(px, textureIndex)];
		}
		case NoiseTypes::Perlin:
		{
			px = (px * (1U << octave));
			return PerlinNoise(px, _DisplayCSCB.PerlinCellSize, octave);
		}
		case NoiseTypes::R2:
		{
			px = (px * (1U << octave));
			if (_DisplayCSCB.DifferentNoisePerOctave && octave > 0)
			{
				uint rng = wang_hash_init(uint3(1337, 255, _DisplayCSCB.RNGSeed ^ octave));
				px += uint2(wang_hash_uint(rng) % 512, wang_hash_uint(rng) % 512);
			}
			return R2LDG(px);
		}
		case NoiseTypes::IGN:
		{
			px = (px * (1U << octave));
			if (_DisplayCSCB.DifferentNoisePerOctave && octave > 0)
			{
				uint rng = wang_hash_init(uint3(1337, 255, _DisplayCSCB.RNGSeed ^ octave));
				px += uint2(wang_hash_uint(rng) % 512, wang_hash_uint(rng) % 512);
			}
			return IGNLDG(px);
		}
	}

	return 0.0f;
}

[numthreads(8, 8, 1)]
#line 168
void csmain(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;
	float ret = 0.0f;

	float totalWeight = 0.0f;
	for (uint octave = 0; octave < _DisplayCSCB.NumberOfOctaves; ++octave)
	{
		float weight = 1.0f / float(1U << octave);
		ret += SampleNoiseTexture(px, octave) * weight;
		totalWeight += weight;
	}
	ret /= totalWeight;

	// remap perlin using PerlinMinMax, then clip anything below 0.
	if (_DisplayCSCB.NoiseType == NoiseTypes::Perlin)
	{
		float percent = (ret - _DisplayCSCB.PerlinMinMax.x) / (_DisplayCSCB.PerlinMinMax.y - _DisplayCSCB.PerlinMinMax.x);
		ret = max(percent, 0.0f);
	}

	Output[px] = ret.x;
	OutputF[px] = ret.x;
}

/*
Shader Resources:
	Texture Output (as UAV)
*/
