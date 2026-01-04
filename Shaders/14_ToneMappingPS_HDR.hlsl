#include "14_Shared.hlsli"

float4 main(PS_INPUT_QUAD input) : SV_Target
{
    //  1. 렌더링된 HDR 색 로드
    float3 C_linear709 = gSceneHDR.Sample(gSamplerLinear, input.uv).rgb;
    float3 C_exposure = C_linear709 * pow(2.0f, gExposure);                
    
    float3 C_final;
    if (gUseWideGamut)  //모니터가 넓은 색역을 지원하는 경우만 
    {
       // 2. 색역 변환 (709 -> 2020) - 반드시 선형(Linear) 상태에서 수행
        float3 C_rec2020 = Rec709ToRec2020(C_exposure);

        // 3. HDR 톤매핑 (Rec.2020 공간의 에너지를 타겟 니트에 맞게 압축)
        float3 nits;
        if (gUseToneMapping)
            nits = ToneMapHDR_Reinhard_Luminance(C_rec2020, gReferenceWhiteNit, gMaxHDRNits);
        else
            nits = C_rec2020 * gReferenceWhiteNit;

        // 4. PQ 인코딩 (디스플레이 신호 규격으로 변환). 10000 nits 기준으로 정규화
        float3 L = nits / 10000.0;   
        C_final = LinearToST2084(L);
    }
    else
    {
        if (gUseToneMapping)
            C_final = ACESFilm(C_exposure);
        else
            C_final = C_exposure;
        
        // Standard Gamut 모니터: Rec.709 유지 (색 왜곡 방지)
        C_final = LinearToSRGB(C_final);
    }  
    // 최종 PQ 인코딩된 값 [0.0, 1.0]을 R10G10B10A2_UNORM 백버퍼에 출력
    return float4(C_final, 1.0f);
}
