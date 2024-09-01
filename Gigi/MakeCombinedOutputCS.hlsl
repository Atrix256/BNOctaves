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

/*
TODO:
* make sure this works with the RELEASED gigi. you have "viewer user file v2" for instance.
* add perlin noise as a noise option.
* generate DX12 app code for this demo and share that too?

Blog:
* where to get gigi and the tutorial
* the repo of this technique, which includes the Gigi files, as well as generated dx12.
* one of the coolest features of Gigi IMO is that you can replace a child graph parameter with a constant value from the parent graph, which makes it a literal instead of a variable. I did this with the channel dot product, and combined the 2 subgraph settings into 1 in the parent.

*/