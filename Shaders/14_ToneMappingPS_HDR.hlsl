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

// PQ는 10,000nit 기준 이므로
// color_for_PQ = linear01 * (displayMaxNits / 10000.0)
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
    float3 C_exposure = C_linear709 * pow(2.0f, gExposure);    
    float3 C_tonemapped = ACESFilm(C_exposure);
  
    const float st2084max = 10000.0;
    const float hdrScalar = gMaxHDRNits / st2084max;
    float3 C_Rec2020 = Rec709ToRec2020(C_tonemapped); 
    float3 C_ST2084 = LinearToST2084(C_Rec2020 * hdrScalar);
    
    // 최종 PQ 인코딩된 값 [0.0, 1.0]을 R10G10B10A2_UNORM 백버퍼에 출력
    return float4(C_ST2084, 1.0);
}