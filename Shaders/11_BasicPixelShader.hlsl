#include "11_Shared.hlsli"

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 main(PS_INPUT input) : SV_Target
{
    return txDiffuse.Sample(samLinear, input.Tex);
}
