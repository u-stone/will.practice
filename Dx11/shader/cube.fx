//[numthreads(1, 1, 1)]
//void main( uint3 DTid : SV_DispatchThreadID )
//{
//}

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
}

//--------------------------------------------------------------------------------------
struct VS_OUTPUT {
    float4 Pos : SV_POSITION;
    float4 Color : COLOR0;
};

struct VS_INPUT {
    float4 Pos : POSITION;
    float4 Color : COLOR;
};

struct PS_INPUT {
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
//VS_OUTPUT VS(float4 Pos : POSITION, float4 Color : COLOR)
PS_INPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Pos = mul(input.Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    output.Color = input.Color;
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input)
    : SV_Target
{
    return input.Color;
}
