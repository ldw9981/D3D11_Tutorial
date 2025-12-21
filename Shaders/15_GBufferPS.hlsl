#include "15_Shared.hlsli"

struct GBufferOut
{
    float4 Albedo   : SV_Target0;
    float4 Normal   : SV_Target1;
    float4 Position : SV_Target2;
};

GBufferOut main(VS_OUTPUT_GBUFFER input)
{
    GBufferOut o;

    // Albedo in linear space
    o.Albedo = gAlbedo;

    // Encode normal from [-1,1] to [0,1]
    float3 n = normalize(input.normalVS);
    o.Normal = float4(EncodeNormal(n), 1.0f);

    // View-space position
    o.Position = float4(input.positionVS, 1.0f);

    return o;
}
