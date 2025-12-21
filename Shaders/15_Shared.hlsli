
// �ε����� �򰥸����־ �Ѱ��� ��Ƶ�
cbuffer CBGeometry : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 BaseColor;
}
cbuffer CBDirectionalLight : register(b1)
{
    float4 gDirLightDirectionWS_Int; // xyz direction in world space, w intensity
    float4 gDirLightColor; // rgb color ,  w intensity
}

cbuffer CBPointLight : register(b2)
{
    float4 gLightPosWS_Radius; // xyz posWS, w radius
    float4 gLightColor; // rgb color
}

cbuffer CBScreenSize : register(b3)
{
    float2 gScreenSize;
    float2 padding2;
}

cbuffer CBCameraPos : register(b4)
{
    float3 gCameraPosWS;
    float padding3;
}


Texture2D gGBufferBaseColor : register(t0);
Texture2D gGBufferNormal   : register(t1);
Texture2D gGBufferPosition : register(t2);
Texture2D gDepthBuffer : register(t3);
SamplerState gSamplerLinear : register(s0);

struct VS_INPUT_CUBE
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
};

struct VS_OUTPUT_GBUFFER
{
    float4 positionCS : SV_Position;
    float3 normalWS   : TEXCOORD0;
    float3 positionWS : TEXCOORD1;
};

struct VS_INPUT_QUAD
{
    float3 position : POSITION;
    float2 uv       : TEXCOORD0;
};

struct PS_INPUT_QUAD
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

float3 DecodeNormal(float3 enc)
{
    // enc is in [0,1], decode to [-1,1]
    return normalize(enc * 2.0f - 1.0f);
}

float3 EncodeNormal(float3 n)
{
    // n is in [-1,1], encode to [0,1]
    return n * 0.5f + 0.5f;
}

float3 LinearToSRGB(float3 linearColor)
{
    return pow(saturate(linearColor), 1.0f / 2.2f);
}
