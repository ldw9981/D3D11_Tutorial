// Fullscreen Quad Vertex Shader
struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

VSOutput main(uint id : SV_VertexID)
{
    VSOutput output;
    
    // Generate fullscreen triangle
    output.TexCoord = float2((id << 1) & 2, id & 2);
    output.Position = float4(output.TexCoord * float2(2, -2) + float2(-1, 1), 0, 1);
    
    return output;
}
