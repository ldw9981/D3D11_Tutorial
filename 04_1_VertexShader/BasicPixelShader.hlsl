// «»ºø ºŒ¿Ã¥ı(Ω¶¿Ã¥ı/ºŒ¿Ã¥ı).

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR0;
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 main(PS_INPUT input) : SV_Target
{
    return input.Color;
}
