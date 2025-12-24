#include "12_Common.hlsli"

float4 main(VS_OUTPUT input) : SV_TARGET
{
    return ObjTexture.Sample( ObjSamplerState, input.TexCoord );
}
