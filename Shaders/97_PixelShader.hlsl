cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 vLightDir;
    float4 vLightColor;
    float4 vMaterialColor;
}

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
};

float4 main(PS_INPUT input) : SV_Target
{
    float3 lightDir = normalize(-vLightDir.xyz);
    float3 normal = normalize(input.Normal);
    float NdotL = max(0.0f, dot(normal, lightDir));
    
    float3 color = vMaterialColor.rgb * vLightColor.rgb * NdotL;
    
    return float4(color, vMaterialColor.a);
}
