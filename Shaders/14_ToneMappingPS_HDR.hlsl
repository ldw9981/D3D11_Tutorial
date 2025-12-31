#include "14_Shared.hlsli"

float4 main(PS_INPUT_QUAD input) : SV_Target
{
    //  렌더링된 HDR 색 로드
    float3 C_linear709 = gSceneHDR.Sample(gSamplerLinear, input.uv).rgb;

    // Exposure 적용
    float3 C_exposure = C_linear709 * pow(2.0f, gExposure);            
    
    // 색역 변환: 모니터가 넓은 색역을 지원하는 경우만 Rec.2020으로 변환
    float3 C_final;
    if (gUseWideGamut)
    {      
        float3 nits = ToneMapHDR_Reinhard(C_linear709, gReferenceWhiteNit, gMaxHDRNits);
        // 10000 nits 기준으로 정규화 후 PQ 인코딩
        float3 L = nits / 10000.0;   
        C_final = LinearToST2084(L);
    }
    else
    {
        float3 C_tonemapped = ACESFilm(C_exposure);
        // Standard Gamut 모니터: Rec.709 유지 (색 왜곡 방지)
        C_final = LinearToSRGB(C_tonemapped);
    }  
    // 최종 PQ 인코딩된 값 [0.0, 1.0]을 R10G10B10A2_UNORM 백버퍼에 출력
    return float4(C_final, 1.0f);
}
