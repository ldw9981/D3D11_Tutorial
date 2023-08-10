// ¡§¡° ºŒ¿Ã¥ı.

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

VS_OUTPUT main(float4 pos : POSITION, float4 color : COLOR)
{
    VS_OUTPUT output;
    output.pos = pos;
    output.color = color;
    return output;
}