cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 vLightDir;
    float4 vLightColor;
    float4 vObjectColor;
}

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : TEXCOORD0;
    float4 Color : COLOR;
};

float4 main(PS_INPUT input) : SV_Target
{
    float3 normalizedNormal = normalize(input.Normal);
    float3 normalizedLightDir = normalize(-vLightDir.xyz);

    float diffuse = saturate(dot(normalizedNormal, normalizedLightDir));

    float3 ambient = float3(0.2f, 0.2f, 0.2f);
    float3 finalColor = vObjectColor.rgb * (ambient + diffuse * vLightColor.rgb);

    return float4(finalColor, vObjectColor.a);
}
