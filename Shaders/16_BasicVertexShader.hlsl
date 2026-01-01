#include "14_Shared.hlsli"

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT_BASIC main(VS_INPUT_BASIC input)
{
    PS_INPUT_BASIC output = (PS_INPUT_BASIC) 0;
    output.position = mul(input.position, World);
    output.position = mul(output.position, View);
    output.position = mul(output.position, Projection);
    
    output.normal = normalize(mul(input.normal, (float3x3) World)); 
    return output;
}
