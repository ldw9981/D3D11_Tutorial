// FXAA (Fast Approximate Anti-Aliasing) Pixel Shader
// NVIDIA FXAA 3.11 - PC Quality Preset

Texture2D sceneTexture : register(t0);
SamplerState linearSampler : register(s0);

cbuffer FXAAParams : register(b0)
{
    float fxaaQualitySubpix;           // 0.0 to 1.0 (default: 0.75)
    float fxaaQualityEdgeThreshold;    // 0.063 to 0.333 (default: 0.166)
    float fxaaQualityEdgeThresholdMin; // 0.0312 to 0.0833 (default: 0.0833)
    float padding;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

float FxaaLuma(float3 rgb) 
{
    return rgb.y * (0.587/0.299) + rgb.x;
}

float4 FxaaPixelShader(float2 pos, Texture2D tex, SamplerState samp, float2 rcpFrame, float QualitySubpix, float QualityEdgeThreshold, float QualityEdgeThresholdMin)
{
    float2 posM = pos;
    float4 rgbyM = tex.Sample(samp, posM);
    float lumaM = FxaaLuma(rgbyM.rgb);
    
    float lumaS = FxaaLuma(tex.SampleLevel(samp, posM, 0.0, int2(0, 1)).rgb);
    float lumaE = FxaaLuma(tex.SampleLevel(samp, posM, 0.0, int2(1, 0)).rgb);
    float lumaN = FxaaLuma(tex.SampleLevel(samp, posM, 0.0, int2(0, -1)).rgb);
    float lumaW = FxaaLuma(tex.SampleLevel(samp, posM, 0.0, int2(-1, 0)).rgb);
    
    float maxSM = max(lumaS, lumaM);
    float minSM = min(lumaS, lumaM);
    float maxESM = max(lumaE, maxSM);
    float minESM = min(lumaE, minSM);
    float maxWN = max(lumaN, lumaW);
    float minWN = min(lumaN, lumaW);
    float rangeMax = max(maxWN, maxESM);
    float rangeMin = min(minWN, minESM);
    float rangeMaxScaled = rangeMax * QualityEdgeThreshold;
    float range = rangeMax - rangeMin;
    float rangeMaxClamped = max(QualityEdgeThresholdMin, rangeMaxScaled);
    
    bool earlyExit = range < rangeMaxClamped;
    if(earlyExit)
        return rgbyM;
    
    float lumaNW = FxaaLuma(tex.SampleLevel(samp, posM, 0.0, int2(-1, -1)).rgb);
    float lumaNE = FxaaLuma(tex.SampleLevel(samp, posM, 0.0, int2(1, -1)).rgb);
    float lumaSW = FxaaLuma(tex.SampleLevel(samp, posM, 0.0, int2(-1, 1)).rgb);
    float lumaSE = FxaaLuma(tex.SampleLevel(samp, posM, 0.0, int2(1, 1)).rgb);
    
    float lumaNS = lumaN + lumaS;
    float lumaWE = lumaW + lumaE;
    float subpixRcpRange = 1.0 / range;
    float subpixNSWE = lumaNS + lumaWE;
    float edgeHorz1 = (-2.0 * lumaM) + lumaNS;
    float edgeVert1 = (-2.0 * lumaM) + lumaWE;
    
    float lumaNESE = lumaNE + lumaSE;
    float lumaNWNE = lumaNW + lumaNE;
    float edgeHorz2 = (-2.0 * lumaE) + lumaNESE;
    float edgeVert2 = (-2.0 * lumaN) + lumaNWNE;
    
    float lumaNWSW = lumaNW + lumaSW;
    float lumaSWSE = lumaSW + lumaSE;
    float edgeHorz4 = (abs(edgeHorz1) * 2.0) + abs(edgeHorz2);
    float edgeVert4 = (abs(edgeVert1) * 2.0) + abs(edgeVert2);
    float edgeHorz3 = (-2.0 * lumaW) + lumaNWSW;
    float edgeVert3 = (-2.0 * lumaS) + lumaSWSE;
    float edgeHorz = abs(edgeHorz3) + edgeHorz4;
    float edgeVert = abs(edgeVert3) + edgeVert4;
    
    float subpixNWSWNESE = lumaNWSW + lumaNESE;
    float lengthSign = rcpFrame.x;
    bool horzSpan = edgeHorz >= edgeVert;
    float subpixA = subpixNSWE * 2.0 + subpixNWSWNESE;
    
    if(!horzSpan) lumaN = lumaW;
    if(!horzSpan) lumaS = lumaE;
    if(horzSpan) lengthSign = rcpFrame.y;
    float subpixB = (subpixA * (1.0/12.0)) - lumaM;
    
    float gradientN = lumaN - lumaM;
    float gradientS = lumaS - lumaM;
    float lumaNN = lumaN + lumaM;
    float lumaSS = lumaS + lumaM;
    bool pairN = abs(gradientN) >= abs(gradientS);
    float gradient = max(abs(gradientN), abs(gradientS));
    if(pairN) lengthSign = -lengthSign;
    float subpixC = saturate(abs(subpixB) * subpixRcpRange);
    
    float2 posB;
    posB.x = posM.x;
    posB.y = posM.y;
    float2 offNP;
    offNP.x = (!horzSpan) ? 0.0 : rcpFrame.x;
    offNP.y = (horzSpan) ? 0.0 : rcpFrame.y;
    if(!horzSpan) posB.x += lengthSign * 0.5;
    if(horzSpan) posB.y += lengthSign * 0.5;
    
    float2 posN;
    posN.x = posB.x - offNP.x * 1.0;
    posN.y = posB.y - offNP.y * 1.0;
    float2 posP;
    posP.x = posB.x + offNP.x * 1.0;
    posP.y = posB.y + offNP.y * 1.0;
    float subpixD = ((-2.0) * subpixC) + 3.0;
    float lumaEndN = FxaaLuma(tex.Sample(samp, posN).rgb);
    float subpixE = subpixC * subpixC;
    float lumaEndP = FxaaLuma(tex.Sample(samp, posP).rgb);
    
    if(!pairN) lumaNN = lumaSS;
    float gradientScaled = gradient * 1.0/4.0;
    float lumaMM = lumaM - lumaNN * 0.5;
    float subpixF = subpixD * subpixE;
    bool lumaMLTZero = lumaMM < 0.0;
    
    lumaEndN -= lumaNN * 0.5;
    lumaEndP -= lumaNN * 0.5;
    bool doneN = abs(lumaEndN) >= gradientScaled;
    bool doneP = abs(lumaEndP) >= gradientScaled;
    if(!doneN) posN.x -= offNP.x * 1.5;
    if(!doneN) posN.y -= offNP.y * 1.5;
    bool doneNP = (!doneN) || (!doneP);
    if(!doneP) posP.x += offNP.x * 1.5;
    if(!doneP) posP.y += offNP.y * 1.5;
    
    if(doneNP) {
        if(!doneN) lumaEndN = FxaaLuma(tex.Sample(samp, posN.xy).rgb);
        if(!doneP) lumaEndP = FxaaLuma(tex.Sample(samp, posP.xy).rgb);
        if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
        if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
        doneN = abs(lumaEndN) >= gradientScaled;
        doneP = abs(lumaEndP) >= gradientScaled;
        if(!doneN) posN.x -= offNP.x * 2.0;
        if(!doneN) posN.y -= offNP.y * 2.0;
        doneNP = (!doneN) || (!doneP);
        if(!doneP) posP.x += offNP.x * 2.0;
        if(!doneP) posP.y += offNP.y * 2.0;
        
        if(doneNP) {
            if(!doneN) lumaEndN = FxaaLuma(tex.Sample(samp, posN.xy).rgb);
            if(!doneP) lumaEndP = FxaaLuma(tex.Sample(samp, posP.xy).rgb);
            if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
            if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
            doneN = abs(lumaEndN) >= gradientScaled;
            doneP = abs(lumaEndP) >= gradientScaled;
            if(!doneN) posN.x -= offNP.x * 2.0;
            if(!doneN) posN.y -= offNP.y * 2.0;
            doneNP = (!doneN) || (!doneP);
            if(!doneP) posP.x += offNP.x * 2.0;
            if(!doneP) posP.y += offNP.y * 2.0;
            
            if(doneNP) {
                if(!doneN) lumaEndN = FxaaLuma(tex.Sample(samp, posN.xy).rgb);
                if(!doneP) lumaEndP = FxaaLuma(tex.Sample(samp, posP.xy).rgb);
                if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
                if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
                doneN = abs(lumaEndN) >= gradientScaled;
                doneP = abs(lumaEndP) >= gradientScaled;
                if(!doneN) posN.x -= offNP.x * 4.0;
                if(!doneN) posN.y -= offNP.y * 4.0;
                doneNP = (!doneN) || (!doneP);
                if(!doneP) posP.x += offNP.x * 4.0;
                if(!doneP) posP.y += offNP.y * 4.0;
                
                if(doneNP) {
                    if(!doneN) lumaEndN = FxaaLuma(tex.Sample(samp, posN.xy).rgb);
                    if(!doneP) lumaEndP = FxaaLuma(tex.Sample(samp, posP.xy).rgb);
                    if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
                    if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
                    doneN = abs(lumaEndN) >= gradientScaled;
                    doneP = abs(lumaEndP) >= gradientScaled;
                    if(!doneN) posN.x -= offNP.x * 8.0;
                    if(!doneN) posN.y -= offNP.y * 8.0;
                    if(!doneP) posP.x += offNP.x * 8.0;
                    if(!doneP) posP.y += offNP.y * 8.0;
                }
            }
        }
    }
    
    float dstN = posM.x - posN.x;
    float dstP = posP.x - posM.x;
    if(!horzSpan) dstN = posM.y - posN.y;
    if(!horzSpan) dstP = posP.y - posM.y;
    
    bool goodSpanN = (lumaEndN < 0.0) != lumaMLTZero;
    float spanLength = (dstP + dstN);
    bool goodSpanP = (lumaEndP < 0.0) != lumaMLTZero;
    float spanLengthRcp = 1.0 / spanLength;
    
    bool directionN = dstN < dstP;
    float dst = min(dstN, dstP);
    bool goodSpan = directionN ? goodSpanN : goodSpanP;
    float subpixG = subpixF * subpixF;
    float pixelOffset = (dst * (-spanLengthRcp)) + 0.5;
    float subpixH = subpixG * QualitySubpix;
    
    float pixelOffsetGood = goodSpan ? pixelOffset : 0.0;
    float pixelOffsetSubpix = max(pixelOffsetGood, subpixH);
    if(!horzSpan) posM.x += pixelOffsetSubpix * lengthSign;
    if(horzSpan) posM.y += pixelOffsetSubpix * lengthSign;
    
    return float4(tex.Sample(samp, posM).rgb, rgbyM.a);
}

float4 main(PSInput input) : SV_TARGET
{
    float2 texSize;
    sceneTexture.GetDimensions(texSize.x, texSize.y);
    float2 rcpFrame = 1.0 / texSize;
    
    return FxaaPixelShader(
        input.TexCoord,
        sceneTexture,
        linearSampler,
        rcpFrame,
        fxaaQualitySubpix,
        fxaaQualityEdgeThreshold,
        fxaaQualityEdgeThresholdMin
    );
}
