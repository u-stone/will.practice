//[numthreads(1, 1, 1)]
//void main( uint3 DTid : SV_DispatchThreadID )
//{
//}

float4 VS(float4 Pos : POSITION) : SV_POSITION
{
    return Pos;
}