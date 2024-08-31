// Unnamed technique, shader InitMinMaxValueCS
/*$(ShaderResources)*/

#define FLT_MAX 3.402823466e+38

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	MinMaxValue[0] = uint2(asuint(FLT_MAX), asuint(0.0f));
}

/*
Shader Resources:
	Buffer MinMaxValue (as UAV)
*/
