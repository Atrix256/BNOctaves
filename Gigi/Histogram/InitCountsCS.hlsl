// Unnamed technique, shader InitCountsCS
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	Counts[DTid.x] = 0;
}

/*
Shader Resources:
	Buffer Counts (as UAV)
*/
