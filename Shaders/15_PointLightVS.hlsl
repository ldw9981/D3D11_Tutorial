#include "15_Shared.hlsli"

VS_OUTPUT_LIGHTVOLUME main(VS_INPUT_CUBE input)
{
    VS_OUTPUT_LIGHTVOLUME output = (VS_OUTPUT_LIGHTVOLUME)0;

    float4 posW = mul(float4(input.position, 1.0f), World);
    float4 posV = mul(posW, View);
    output.positionCS = mul(posV, Projection);

    return output;
}
