#include "15_Shared.hlsli"



float4 main(VS_OUTPUT_GBUFFER input) : SV_Target
{
    // Calculate screen UV from pixel position
    float2 screenUV = input.positionCS.xy / gScreenSize;
    
    // Sample G-Buffer
    float3 baseColor = gGBufferBaseColor.Sample(gSamplerLinear, screenUV).rgb;
    float3 normalEnc = gGBufferNormal.Sample(gSamplerLinear, screenUV).rgb;
    float3 posWS = gGBufferPosition.Sample(gSamplerLinear, screenUV).xyz;

    // Check if there's valid geometry at this pixel
    // G-Buffer normal is cleared to (0,0,0), so if length is near zero, no geometry
    float normalLength = length(normalEnc);
    if (normalLength < 0.01f)
        discard;

    float3 n = DecodeNormal(normalEnc);

    float3 lightPosWS = gLightPosWS_Radius.xyz;
    float radius = gLightPosWS_Radius.w;

    float3 L = lightPosWS - posWS;
    float dist = length(L);
    if (dist > radius)
        discard;
      
    float3 Ldir = L / max(dist, 1e-5f);

    float atten = saturate(1.0f - dist / radius);
    atten *= atten;

    float ndotl = saturate(dot(n, Ldir));
    float3 lightColor = gLightColor.rgb;

    float3 colorLinear = baseColor * lightColor * ndotl * atten;
   
    return float4(colorLinear, 0.0f);
}
