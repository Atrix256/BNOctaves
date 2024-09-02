// Unnamed technique, shader MakeCombinedOutputCS
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
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
