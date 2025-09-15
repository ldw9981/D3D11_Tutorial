#include <05_Shared.hlsli>

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT main(float4 Pos : POSITION, float4 Color : COLOR)
{
    PS_INPUT output = (PS_INPUT) 0;
    output.Pos = mul(Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    output.Color = Color;
    return output;
}


