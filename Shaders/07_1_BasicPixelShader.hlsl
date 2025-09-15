#include "07_1_shared.hlsli"

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 main(PS_INPUT input) : SV_Target
{
    return txDiffuse.Sample(samLinear, input.Tex) * vMeshColor;
}