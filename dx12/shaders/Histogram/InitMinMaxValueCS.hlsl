// Unnamed technique, shader InitMinMaxValueCS


RWStructuredBuffer<uint2> MinMaxValue : register(u0);

#line 2


#define FLT_MAX 3.402823466e+38

[numthreads(8, 8, 1)]
#line 6
void csmain(uint3 DTid : SV_DispatchThreadID)
{
	MinMaxValue[0] = uint2(asuint(FLT_MAX), asuint(0.0f));
}

/*
Shader Resources:
	Buffer MinMaxValue (as UAV)
*/
