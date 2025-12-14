#include "14_Shared.hlsli"


float3 LinearToSRGB(float3 linearColor)
{
    return pow(linearColor, 1.0f / 2.2f);
}

float4 main(PS_INPUT_QUAD input) : SV_Target
{
     // 1. 선형 HDR 값 로드 (Nits 값으로 간주)
    float3 C_linear709 = gSceneHDR.Sample(gSamplerLinear, input.uv).rgb;
    C_linear709 *= gExposure;
    float3 C_tonemapped = ACESFilm(C_linear709);
    float3 C_final;
    C_final = LinearToSRGB(C_tonemapped);
  
    
    return float4(C_final, 1.0);
}
