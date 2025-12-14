#include "14_Shared.hlsli"

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 main(PS_INPUT_QUAD input) : SV_Target
{
    float3 hdr = gSceneHDR.Sample(gSamplerLinear, input.uv).rgb;
    hdr = saturate(hdr);
    return float4(hdr, 1.0);
}
