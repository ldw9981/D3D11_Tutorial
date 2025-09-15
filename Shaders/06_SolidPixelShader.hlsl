#include "06_Shared.hlsli"


//--------------------------------------------------------------------------------------
// PSSolid - render a solid color
//--------------------------------------------------------------------------------------
float4 main(PS_INPUT input) : SV_Target
{
    return vOutputColor;
}
