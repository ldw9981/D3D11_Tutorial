#include "98_Shared.hlsli"

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 main(PS_INPUT input) : SV_Target
{
    // Normalize the normal and light direction
    float3 normal = normalize(input.Normal);
    float3 lightDir = normalize(-vLightDir);
    
    // Calculate diffuse lighting
    float diffuse = max(dot(normal, lightDir), 0.0f);
    
    // Add ambient lighting
    float ambient = 0.3f;
    
    // Combine lighting
    float3 lighting = (ambient + diffuse) * vLightColor;
    
    // Apply lighting to mesh color
    float4 finalColor = vMeshColor;
    finalColor.rgb *= lighting;
    
    return finalColor;
}
