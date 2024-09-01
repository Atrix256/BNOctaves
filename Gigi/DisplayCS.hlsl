// Unnamed technique, shader DisplayCS
/*$(ShaderResources)*/

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

float SampleNoiseTexture(uint2 px, uint octave)
{
	switch(/*$(Variable:NoiseType)*/)
	{
		case NoiseTypes::Blue:
		{
			uint3 dims;
			/*$(Image2DArray:Assets/real_uniform_gauss1_0_Gauss10_separate05_%i.png:R8_Unorm:float:false)*/.GetDimensions(dims.x, dims.y, dims.z);
			px = (px * (1 << octave)) % dims.xy;
			uint textureIndex = /*$(Variable:DifferentNoisePerOctave)*/ ? (octave % dims.z ) : 0;
			return /*$(Image2DArray:Assets/real_uniform_gauss1_0_Gauss10_separate05_%i.png:R8_Unorm:float:false)*/[uint3(px, textureIndex)];
		}		
		case NoiseTypes::White:
		{
			px = (px * (1 << octave));
			uint textureIndex = /*$(Variable:DifferentNoisePerOctave)*/ ? octave : 0;
			uint rng = wang_hash_init(uint3(px, textureIndex));
			return wang_hash_float01(rng);
		}
		case NoiseTypes::Binomial3x3:
		{
			uint3 dims;
			/*$(Image2DArray:Assets/real_uniform_binomial3x3_Gauss10_separate05_%i.png:R8_Unorm:float:false)*/.GetDimensions(dims.x, dims.y, dims.z);
			px = (px * (1 << octave)) % dims.xy;
			uint textureIndex = /*$(Variable:DifferentNoisePerOctave)*/ ? (octave % dims.z ) : 0;
			return /*$(Image2DArray:Assets/real_uniform_binomial3x3_Gauss10_separate05_%i.png:R8_Unorm:float:false)*/[uint3(px, textureIndex)];
		}
		case NoiseTypes::Box3x3:
		{
			uint3 dims;
			/*$(Image2DArray:Assets/real_uniform_box3x3_Gauss10_separate05_%i.png:R8_Unorm:float:false)*/.GetDimensions(dims.x, dims.y, dims.z);
			px = (px * (1 << octave)) % dims.xy;
			uint textureIndex = /*$(Variable:DifferentNoisePerOctave)*/ ? (octave % dims.z ) : 0;
			return /*$(Image2DArray:Assets/real_uniform_box3x3_Gauss10_separate05_%i.png:R8_Unorm:float:false)*/[uint3(px, textureIndex)];
		}
		case NoiseTypes::Box5x5:
		{
			uint3 dims;
			/*$(Image2DArray:Assets/real_uniform_box5x5_Gauss10_separate05_%i.png:R8_Unorm:float:false)*/.GetDimensions(dims.x, dims.y, dims.z);
			px = (px * (1 << octave)) % dims.xy;
			uint textureIndex = /*$(Variable:DifferentNoisePerOctave)*/ ? (octave % dims.z ) : 0;
			return /*$(Image2DArray:Assets/real_uniform_box5x5_Gauss10_separate05_%i.png:R8_Unorm:float:false)*/[uint3(px, textureIndex)];
		}
		case NoiseTypes::R2:
		{
			px = (px * (1 << octave));
			if (/*$(Variable:DifferentNoisePerOctave)*/ && octave > 0)
			{
				uint rng = wang_hash_init(uint3(1337, 255, octave));
				px += uint2(wang_hash_uint(rng) % 512, wang_hash_uint(rng) % 512);
			}
			return R2LDG(px);
		}
		case NoiseTypes::IGN:
		{
			px = (px * (1 << octave));
			if (/*$(Variable:DifferentNoisePerOctave)*/ && octave > 0)
			{
				uint rng = wang_hash_init(uint3(1337, 255, octave));
				px += uint2(wang_hash_uint(rng) % 512, wang_hash_uint(rng) % 512);
			}
			return IGNLDG(px);
		}
	}

	return 0.0f;
}

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;
	float ret = 0.0f;

	float totalWeight = 0.0f;
	for (uint octave = 0; octave < /*$(Variable:NumberOfOctaves)*/; ++octave)
	{
		float weight = 1.0f / float(1 << octave);
		ret += SampleNoiseTexture(px, octave) * weight;
		totalWeight += weight;
	}
	ret /= totalWeight;

	Output[px] = ret.x;
	OutputF[px] = ret.x;
}

/*
Shader Resources:
	Texture Output (as UAV)
*/
/*
TODO:
- make a DFT magnitude subgraph node? could show both the input texture DFT and the output.
 - should let you normalize result, and also option to remove DC
*/