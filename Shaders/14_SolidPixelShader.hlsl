#include "14_Shared.hlsli"


//--------------------------------------------------------------------------------------
// PSSolid - render a solid color
//--------------------------------------------------------------------------------------
float4 main(PS_INPUT_BASIC input) : SV_Target
{
    return vOutputColor;
}
