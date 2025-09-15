#include "06_Shared.hlsli"

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    output.Pos = mul(input.Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    
    // 이 코드는 World Matrix에 스케일 또는 이동행렬이 적용되어 있으면 크기가 1이 아닌 벡터가 될수있다.
    //output.Norm = mul(float4(input.Norm, 1), World).xyz;    
   
    // World Matrix에서 이동성분을 제외하고 적용하며,  scale 있을수 있으므로 normalize 사용한다.
    output.Norm = normalize(mul(input.Norm, (float3x3) World)); 
   
    
    return output;
}