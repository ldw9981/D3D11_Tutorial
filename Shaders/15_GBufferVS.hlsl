#include "15_Shared.hlsli"

VS_OUTPUT_GBUFFER main(VS_INPUT_CUBE input)
{
    VS_OUTPUT_GBUFFER output = (VS_OUTPUT_GBUFFER)0;

    float4 posW = mul(float4(input.position, 1.0f), World);
    float4 posV = mul(posW, View);
    output.positionCS = mul(posV, Projection);

    // World-space normal and position (for light pass)
    float3 nW = normalize(mul(input.normal, (float3x3)World));
    output.normalWS = nW;
    output.positionWS = posW.xyz;

    return output;
}
