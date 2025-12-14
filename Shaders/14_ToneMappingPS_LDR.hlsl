#include "14_Shared.hlsli"


float4 main(PS_INPUT_QUAD input) : SV_Target
{
    float3 hdr = gSceneHDR.Sample(gSamplerLinear, input.uv).rgb;
    return float4(hdr, 1.0);
}
