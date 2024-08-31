// Unnamed technique, shader MakeGraphCS
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;
	float countIndexF = (/*$(Variable:NumBuckets)*/-1) * float(px.x) / float(/*$(Variable:GraphSize)*/.x-1);
	uint countIndex = uint(countIndexF);
	uint count1 = Counts[min(countIndex+1, /*$(Variable:NumBuckets)*/ - 1)];
	uint count2 = Counts[min(countIndex+2, /*$(Variable:NumBuckets)*/ - 1)];
	float count = lerp(float(count1), float(count2), frac(countIndexF));
	float2 yRange = float2(0.0f, Counts[0] * 1.1f);
	float countPercent = (count - yRange.x) / (yRange.y - yRange.x);

	float2 uv = float2(px) / float2(/*$(Variable:GraphSize)*/);
	uv.y = 1.0f - uv.y;

	float shade = (uv.y < countPercent) ? 0.0f : 1.0f;

	Output[px] = float4(shade.xxx, 1.0f);
}

/*
Shader Resources:
	Buffer Counts (as SRV)
	Texture Output (as UAV)
*/
