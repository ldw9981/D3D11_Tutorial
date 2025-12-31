#include "14_Shared.hlsli"




float4 main(PS_INPUT_QUAD input) : SV_Target
{
    // 1. 렌더링된 HDR 색 로드
    float3 C_linear709 = gSceneHDR.Sample(gSamplerLinear, input.uv).rgb;

    // 2. Exposure 적용
    float3 C_exposure = C_linear709 * pow(2.0f, gExposure);

    // 3. 톤매핑 적용 여부
    float3 C_tonemapped;
    if (gUseToneMapping)
    {
        // 톤매핑 적용 (ACES Film)
        C_tonemapped = ACESFilm(C_exposure);
    }
    else
    {
        // 톤매핑 없이 Exposure만 적용
        C_tonemapped = C_exposure;
    }

    // 4. sRGB Gamma 적용 (SDR 필수)
    float3 C_final = LinearToSRGB(C_tonemapped);

    return float4(C_final, 1.0);
}
