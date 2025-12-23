// FXAA (Fast Approximate Anti-Aliasing) Pixel Shader
// Based on NVIDIA FXAA 3.11

Texture2D sceneTexture : register(t0);
SamplerState linearSampler : register(s0);

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

#define FXAA_SPAN_MAX 8.0
#define FXAA_REDUCE_MUL (1.0/FXAA_SPAN_MAX)
#define FXAA_REDUCE_MIN (1.0/128.0)
#define FXAA_SUBPIX_SHIFT (1.0/4.0)

float4 main(PSInput input) : SV_TARGET
{
    float2 uv = input.TexCoord;
    
    // Get texture dimensions
    float2 texSize;
    sceneTexture.GetDimensions(texSize.x, texSize.y);
    float2 rcpFrame = 1.0 / texSize;
    
    float3 rgbNW = sceneTexture.Sample(linearSampler, uv + float2(-1.0, -1.0) * rcpFrame).xyz;
    float3 rgbNE = sceneTexture.Sample(linearSampler, uv + float2(1.0, -1.0) * rcpFrame).xyz;
    float3 rgbSW = sceneTexture.Sample(linearSampler, uv + float2(-1.0, 1.0) * rcpFrame).xyz;
    float3 rgbSE = sceneTexture.Sample(linearSampler, uv + float2(1.0, 1.0) * rcpFrame).xyz;
    float3 rgbM = sceneTexture.Sample(linearSampler, uv).xyz;
    
    float3 luma = float3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM = dot(rgbM, luma);
    
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
    
    float2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));
    
    float dirReduce = max(
        (lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),
        FXAA_REDUCE_MIN);
    
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    
    dir = min(float2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
          max(float2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
          dir * rcpDirMin)) * rcpFrame;
    
    float3 rgbA = 0.5 * (
        sceneTexture.Sample(linearSampler, uv + dir * (1.0 / 3.0 - 0.5)).xyz +
        sceneTexture.Sample(linearSampler, uv + dir * (2.0 / 3.0 - 0.5)).xyz);
    
    float3 rgbB = rgbA * 0.5 + 0.25 * (
        sceneTexture.Sample(linearSampler, uv + dir * -0.5).xyz +
        sceneTexture.Sample(linearSampler, uv + dir * 0.5).xyz);
    
    float lumaB = dot(rgbB, luma);
    
    if ((lumaB < lumaMin) || (lumaB > lumaMax))
        return float4(rgbA, 1.0);
    else
        return float4(rgbB, 1.0);
}
