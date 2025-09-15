#include "03_Shared.hlsli"

float4 main(PS_INPUT input) : SV_TARGET
{
    return input.color;
}