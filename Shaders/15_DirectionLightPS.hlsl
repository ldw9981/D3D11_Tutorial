#include "15_Shared.hlsli"



float4 main(PS_INPUT_QUAD input) : SV_Target
{
    float2 uv = input.uv;  
    
    float3 baseColor = gGBufferBaseColor.Sample(gSamplerLinear, uv).rgb;
    float3 normalEnc = gGBufferNormal.Sample(gSamplerLinear, uv).rgb;
    float3 posWS = gGBufferPosition.Sample(gSamplerLinear, uv).xyz;

    float3 n = DecodeNormal(normalEnc);

    float3 dirLightDirWS = normalize(gDirLightDirectionWS_Int.xyz);
    float intensity = gDirLightDirectionWS_Int.w;

    // Directional light points from surface toward light
    float ndotl = saturate(dot(n, -dirLightDirWS));
    
    float3 lightColor = gDirLightColor.rgb;

    float3 colorLinear = baseColor * lightColor * ndotl * intensity;
    return float4(colorLinear, 1.0f);
}
