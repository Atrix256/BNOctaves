// Unnamed technique, shader InitCountsCS


RWStructuredBuffer<uint> Counts : register(u0);

#line 2


[numthreads(64, 1, 1)]
#line 4
void csmain(uint3 DTid : SV_DispatchThreadID)
{
	Counts[DTid.x] = 0;
}

/*
Shader Resources:
	Buffer Counts (as UAV)
*/
