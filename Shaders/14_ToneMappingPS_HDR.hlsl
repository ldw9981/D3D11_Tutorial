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
    //  렌더링된 HDR 색 로드
    float3 C_linear709 = gSceneHDR.Sample(gSamplerLinear, input.uv).rgb;

    // Exposure 적용
    float3 C_exposure = C_linear709 * pow(2.0f, gExposure);

    float3 C_tonemapped;
    if (gUseToneMapping)
    {
        C_tonemapped = ACESFilm(C_exposure);
    }
    else
    {
        C_tonemapped = C_exposure;
    }
    
    // 색역 변환: 모니터가 넓은 색역을 지원하는 경우만 Rec.2020으로 변환
    float3 C_final;
    if (gUseWideGamut)
    {
        // Wide Gamut 모니터: Rec.709 -> Rec.2020 변환
        float3 C_Gamut = Rec709ToRec2020(C_tonemapped);

        const float st2084max = 10000.0;
        float3 finalColorNits = C_Gamut * gReferenceWhiteNit;
        finalColorNits = min(finalColorNits, gMaxHDRNits);

    // ACES 출력 1.0 (가장 밝은 값) → 1500 nits (모니터 최대 밝기) → 0.15 (PQ 정규화)
    // 모니터최대밝기/10000 이 입력할수 있는 한계값 -> 출력도 한계
    // → LinearToST2084(0.15) ≈ 0.58 (최종 출력, R10G10B10A2 포맷으로 저장)   
        C_final = LinearToST2084(finalColorNits / st2084max);        
    }
    else
    {
        // Standard Gamut 모니터: Rec.709 유지 (색 왜곡 방지)
        C_final = LinearToSRGB(C_tonemapped);
    }  
    // 최종 PQ 인코딩된 값 [0.0, 1.0]을 R10G10B10A2_UNORM 백버퍼에 출력
    return float4(C_final, 1.0f);
}
