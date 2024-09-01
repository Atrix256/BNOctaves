// Unnamed technique, shader MakeMinMaxValueCS
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;

	float value = dot(Input[px].xyzw, float4/*$(Variable:ChannelDotProduct)*/);
	uint valueU = asuint(value);
	
	if (/*$(Variable:AutoXAxisRange)*/)
	{
		uint dummy;
		InterlockedMin(MinMaxValue[0].x, valueU, dummy);
		InterlockedMax(MinMaxValue[0].y, valueU, dummy);
	}
	else
	{
		MinMaxValue[0] = uint2(asuint(/*$(Variable:XAxisRange)*/.x), asuint(/*$(Variable:XAxisRange)*/.y));
	}
}

/*
Shader Resources:
	Texture Input (as SRV)
	Texture MinMaxValue (as UAV)
*/
