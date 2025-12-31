
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

float3 ToneMapHDR_Reinhard(float3 sceneLinear, float paperWhiteNits, float peakNits)
{
    // 1) Convert relative scene-linear -> absolute nits anchored at paper white
    float3 nits = sceneLinear * paperWhiteNits;

    // 2) Roll-off to peak (compress highlights smoothly)
    //    This is a "peak-aware Reinhard" style curve:
    //    - stays close to linear around low/mid
    //    - compresses as it approaches peak
    float3 x = nits / peakNits; // normalize to peak
    float3 y = x / (1.0 + x); // compress to < 1
    float3 outNits = y * peakNits; // back to nits

    return outNits;
}

float3 Rec709ToRec2020(float3 color)
{
    static const float3x3 conversion =
    {
        0.627402, 0.329292, 0.043306,
        0.069095, 0.919544, 0.011360,
        0.016394, 0.088028, 0.895578
    };
    return mul(conversion, color);
}

// PQ는 10,000nit 기준 이므로
// color_for_PQ = linear01 * (displayMaxNits / 10000.0)
float3 LinearToST2084(float3 color)
{
    float m1 = 2610.0 / 4096.0 / 4;
    float m2 = 2523.0 / 4096.0 * 128;
    float c1 = 3424.0 / 4096.0;
    float c2 = 2413.0 / 4096.0 * 32;
    float c3 = 2392.0 / 4096.0 * 32;
    float3 cp = pow(abs(color), m1);
    return pow((c1 + c2 * cp) / (1 + c3 * cp), m2);
}
