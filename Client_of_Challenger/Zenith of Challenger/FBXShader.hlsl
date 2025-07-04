#include "Shaders.hlsl"

struct VSInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float3 WorldPos : WORLDPOS;
    float4 shadowPos : TEXCOORD1;
};

PSInput VSMain(VSInput input)
{
    PSInput output;

    float4 worldPosition = mul(float4(input.Position, 1.0f), g_worldMatrix);
    float4 viewPosition = mul(worldPosition, g_viewMatrix);
    output.Position = mul(viewPosition, g_projectionMatrix);

    output.WorldPos = worldPosition.xyz;
    output.Normal = normalize(mul(input.Normal, (float3x3) g_worldMatrix));
    output.TexCoord = float2(input.TexCoord.x, 1.0f - input.TexCoord.y); // UV 뒤집기
    
    

    output.shadowPos = mul(worldPosition, shadowMat);

    return output;
}

//float4 PSMain(PSInput input) : SV_TARGET
//{
//    //float4 texColor = g_texture[g_textureIndex].Sample(g_sampler, input.TexCoord);
//    float4 texColor = g_texture[0].Sample(g_sampler, input.TexCoord);

//    // fallback color in case texture is black
//    if (texColor.r + texColor.g + texColor.b < 0.01f)
//    {
//        texColor.rgb = float3(0.5f, 1.0f, 0.5f); // Magenta for debugging
//    }

//    float3 normal = normalize(input.Normal);
//    float3 toEye = normalize(g_cameraPosition - input.WorldPos);

//    // 기존 lighting 말고 toon lighting 사용
//    float shadow = ComputeShadowFactor(input.shadowPos.xyz);
    
//    return ToonLighting(input.WorldPos, normal, toEye, texColor) * shadow;
//    //float4 debugShadow = ComputeShadowFactor(input.WorldPos);
//    //return debugShadow;
    
//}

float4 PSMain(PSInput input) : SV_TARGET
{
    float4 texColor;

    if (g_useTexture == 1)
    {
        texColor = g_texture[0].Sample(g_sampler, input.TexCoord);
        
        // fallback: 텍스처가 거의 검정이면 디버그 컬러 표시
        if (texColor.r + texColor.g + texColor.b < 0.01f)
            texColor.rgb = float3(1.0f, 0.0f, 1.0f); // Magenta
    }
    else
    {
        texColor = g_baseColor;
    }

    float3 normal = normalize(input.Normal);
    float3 toEye = normalize(g_cameraPosition - input.WorldPos);

    float shadow = ComputeShadowFactor(input.shadowPos.xyz);
    
    return ToonLighting(input.WorldPos, normal, toEye, texColor) * shadow;
}