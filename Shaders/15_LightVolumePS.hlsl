#include "15_Shared.hlsli"



float4 main(VS_OUTPUT_GBUFFER input) : SV_Target
{
    // Calculate screen UV from pixel position
    float2 screenUV = input.positionCS.xy / gScreenSize;
    
    // Sample G-Buffer
    float3 baseColor = gGBufferBaseColor.Sample(gSamplerLinear, screenUV).rgb;
    float3 normalEnc = gGBufferNormal.Sample(gSamplerLinear, screenUV).rgb;
    float3 posVS = gGBufferPosition.Sample(gSamplerLinear, screenUV).xyz;

    // Check if there's valid geometry at this pixel
    // G-Buffer normal is cleared to (0,0,0), so if length is near zero, no geometry
    float normalLength = length(normalEnc);
    if (normalLength < 0.01f)
        discard;

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
    
    // Debug mode: Show light radius boundary
    /*       
    {
        // Highlight pixels near the radius boundary (95% ~ 105%)
        float normalizedDist = dist / radius;
        if (normalizedDist >= 0.99f && normalizedDist <= 1.0f)
        {
            // Red highlight at boundary
            colorLinear = lightColor;
        }
    }
    */
    return float4(LinearToSRGB(colorLinear), 1.0f);
}
