#include "15_Shared.hlsli"



float4 main(PS_INPUT_QUAD input) : SV_Target
{
    float2 uv = input.uv;  
    
    float3 baseColor = gGBufferBaseColor.Sample(gSamplerLinear, uv).rgb;
    float3 normalEnc = gGBufferNormal.Sample(gSamplerLinear, uv).rgb;
    float3 posVS = gGBufferPosition.Sample(gSamplerLinear, uv).xyz;

    float3 n = DecodeNormal(normalEnc);

    float3 lightPosVS = gLightPosVS_Radius.xyz;
    float radius = gLightPosVS_Radius.w;

    float3 L = lightPosVS - posVS;
    float dist = length(L);
    float3 Ldir = (dist > 1e-5f) ? (L / dist) : float3(0, 0, 1);

    float atten = saturate(1.0f - dist / radius);
    atten *= atten;

    float ndotl = saturate(dot(n, Ldir));      
    float3 lightColor = gLightColor.rgb;

    float3 colorLinear = baseColor * lightColor * ndotl * atten;
    return float4(colorLinear, 1.0f);
}
