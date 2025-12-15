#include "14_Shared.hlsli"

float3 Rec709ToRec2020(float3 color)
{
    static const float3x3 conversion =
    {
        0.627402, 0.329292, 0.043306,
        0.069095, 0.919544, 0.011360,
        0.016394, 0.088028, 0.895578
    };
    return mul(conversion, color);
}

//최종 출력하고 싶은 Nits 값을 10000.0 으로 나눈 값을 넣어주어야 합니다.
float3 LinearToST2084(float3 color)
{
    float m1 = 2610.0 / 4096.0 / 4;
    float m2 = 2523.0 / 4096.0 * 128;
    float c1 = 3424.0 / 4096.0;
    float c2 = 2413.0 / 4096.0 * 32;
    float c3 = 2392.0 / 4096.0 * 32;
    float3 cp = pow(abs(color), m1);
    return pow((c1 + c2 * cp) / (1 + c3 * cp), m2);
}

float4 main(PS_INPUT_QUAD input) : SV_Target
{
     // 1. 선형 HDR 값 로드 (Nits 값으로 간주)
    float3 C_linear709 = gSceneHDR.Sample(gSamplerLinear, input.uv).rgb;  
   
    float exposureFactor = pow(2.0f, gExposure);
    C_linear709 *= exposureFactor;
    
    float3 C_tonemapped;
    C_tonemapped = ACESFilm(C_linear709);   
  
    const float st2084max = 10000.0;
    const float hdrScalar = gMaxHDRNits / st2084max;
    float3 result;
    result = Rec709ToRec2020(C_tonemapped);

    // Apply the ST.2084 curve to the scene.
    result = LinearToST2084(result * hdrScalar);
    
    // 최종 PQ 인코딩된 값 [0.0, 1.0]을 R10G10B10A2_UNORM 백버퍼에 출력
    return float4(result, 1.0);
}