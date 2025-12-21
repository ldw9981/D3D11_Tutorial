#include "15_Shared.hlsli"

float4 main(VS_OUTPUT_GBUFFER input) : SV_Target
{
    // Use the base color from constant buffer (for light indicators)
    return BaseColor;
}
