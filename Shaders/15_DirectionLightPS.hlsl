#include "15_Shared.hlsli"

cbuffer CBDirectionalLight : register(b0)
{
    float4 gDirLightDirectionVS_Int;  // xyz direction in view space, w intensity
    float4 gDirLightColor;            // rgb color
}

float4 main(PS_INPUT_QUAD input) : SV_Target
{
    float2 uv = input.uv;  
    
    float3 baseColor = gGBufferBaseColor.Sample(gSamplerLinear, uv).rgb;
    float3 normalEnc = gGBufferNormal.Sample(gSamplerLinear, uv).rgb;
    float3 posVS = gGBufferPosition.Sample(gSamplerLinear, uv).xyz;

    float3 n = DecodeNormal(normalEnc);

    float3 dirLightDirVS = normalize(gDirLightDirectionVS_Int.xyz);
    float intensity = gDirLightDirectionVS_Int.w;

    // Directional light points from surface toward light
    float ndotl = saturate(dot(n, -dirLightDirVS));
    
    float3 lightColor = gDirLightColor.rgb;

    float3 colorLinear = baseColor * lightColor * ndotl * intensity;
    return float4(colorLinear, 1.0f);
}
