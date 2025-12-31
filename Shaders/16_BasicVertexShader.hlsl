#include "14_Shared.hlsli"

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT_BASIC main(VS_INPUT_BASIC input)
{
    PS_INPUT_BASIC output = (PS_INPUT_BASIC) 0;
    output.position = mul(input.position, World);
    output.position = mul(output.position, View);
    output.position = mul(output.position, Projection);

    // World Matrix에서 이동성분을 제외하고 적용하며,  scale 있을수 있으므로 normalize 사용한다.
    output.normal = normalize(mul(input.normal, (float3x3) World)); 
    return output;
}