
Texture2D gSceneHDR : register(t0);
SamplerState gSamplerLinear : register(s0);

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 vLightDir[2];
    float4 vLightColor[2];
    float4 vOutputColor;
    float  gMaxHDRNits;
    float gExposure;
    float padding[2];
}


//--------------------------------------------------------------------------------------
struct VS_INPUT_BASIC
{
    float4 position : POSITION;
    float3 normal : NORMAL;
};

struct PS_INPUT_BASIC
{
    float4 position : SV_POSITION;
    float3 normal : TEXCOORD0;
};

struct VS_INPUT_QUAD
{
    float3 position : POSITION; // ÀÌ¹Ì NDC
    float2 uv : TEXCOORD0;
};

struct PS_INPUT_QUAD
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};






