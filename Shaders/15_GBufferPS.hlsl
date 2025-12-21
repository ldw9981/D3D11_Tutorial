#include "15_Shared.hlsli"

struct GBufferOut
{
    float4 BaseColor: SV_Target0;
    float4 Normal   : SV_Target1;
    float4 Position : SV_Target2;
};

GBufferOut main(VS_OUTPUT_GBUFFER input)
{
    GBufferOut o;

    // BaseColor in linear space
    o.BaseColor = GeometryColor;

    // Encode normal from [-1,1] to [0,1]
    float3 n = normalize(input.normalWS);
    o.Normal = float4(EncodeNormal(n), 1.0f);

    // World-space position
    o.Position = float4(input.positionWS, 1.0f);

    return o;
}
