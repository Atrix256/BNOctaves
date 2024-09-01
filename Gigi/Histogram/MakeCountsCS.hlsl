// Unnamed technique, shader MakeCountsCS
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;

	float value = dot(Input[px].xyzw, float4/*$(Variable:ChannelDotProduct)*/);
	float valuePercent = (value - asfloat(MinMaxValue[0].x)) / (asfloat(MinMaxValue[0].y) - asfloat(MinMaxValue[0].x));

	uint bucketIndex = uint(valuePercent * float(/*$(Variable:NumBuckets)*/-1));

	// don't count min and max buckets if we aren't supposed to
	if (/*$(Variable:ZeroMinMaxBucket)*/ && (bucketIndex == 0 || bucketIndex == (/*$(Variable:NumBuckets)*/-1)))
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
