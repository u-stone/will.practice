//[numthreads(1, 1, 1)]
//void main( uint3 DTid : SV_DispatchThreadID )
//{
//}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(float4 Pos : SV_POSITION) : SV_Target
{
    return float4(1.0f, 1.0f, 0.0f, 1.0f);    // Yellow, with Alpha = 1
}
