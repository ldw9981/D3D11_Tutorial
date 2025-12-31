
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
    int gUseWideGamut;      // 넓은 색역 사용 여부 (0=Rec.709, 1=Rec.2020)
    int gUseToneMapping;    // 톤매핑 적용 여부 (0=Off, 1=ACES Film)
    float gReferenceWhiteNit; // 기준 화이트 포인트 (nits)
    float padding1;
    float padding2;
    float padding3;
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
    float3 position : POSITION; // 이미 NDC
    float2 uv : TEXCOORD0;
};

struct PS_INPUT_QUAD
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};


// 입력: Linear 공간의 HDR RGB 색상값
// 출력: 0.0 ~ 1.0 범위의 압축된 선형 RGB 값 (float3)
float3 ACESFilm(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate(x * (a * x + b) / (x * (c * x + d) + e));
}

float3 LinearToSRGB(float3 linearColor)
{
    return pow(linearColor, 1.0f / 2.2f);
}
