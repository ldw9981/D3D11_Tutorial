#include "15_Shared.hlsli"

VS_OUTPUT_SOLID main(VS_INPUT_CUBE input)
{
    VS_OUTPUT_SOLID output = (VS_OUTPUT_SOLID)0;

    float4 posW = mul(float4(input.position, 1.0f), World);
    float4 posV = mul(posW, View);
    output.position = mul(posV, Projection);

    return output;
}
