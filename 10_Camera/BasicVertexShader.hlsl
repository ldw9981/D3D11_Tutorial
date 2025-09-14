#include "Shared.fxh"

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    output.Pos = mul(input.Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    
    // �� �ڵ�� World Matrix�� �������� ����Ǿ� ������ ũ�Ⱑ 1�� �ƴ� ���Ͱ� �ɼ��ִ�.
    //output.Norm = mul(float4(input.Norm, 1), World).xyz;    
   
    // World Matrix���� �̵������� �����ϰ� �����ϸ�,  scale ������ �����Ƿ� normalize ����Ѵ�.
    output.Norm = normalize(mul(input.Norm, (float3x3) NormalMatrix));
 
    return output;
}