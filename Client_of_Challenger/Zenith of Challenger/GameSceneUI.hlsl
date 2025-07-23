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
    float2 TexCoord : TEXCOORD;
};

// 단순 투영 및 UV 전달용
PSInput VSMain(VSInput input)
{
    PSInput output;

    float4 worldPos = mul(float4(input.Position, 1.0f), g_worldMatrix);
    float4x4 transformLikeStartScene = 
        {1.8, 0.0, 0.0, 0.0,
         0.0, 2.4, 0.0, 0.0,
         0.0, 0.0, 1.0, 1.0,
         0.0, 0.0, -0.1, 0};
    //float4 viewPos = mul(worldPos, g_viewMatrix);
    //output.Position = mul(viewPos, g_projectionMatrix);
    //output.Position = float4(input.Position, 1.0f);
    output.Position = mul(input.Position, worldPos);
    output.Position = mul(worldPos, transformLikeStartScene);
    
    
    if (g_useCustomUV == 1)
    {
        output.TexCoord = lerp(g_customUV.xy, g_customUV.zw, input.TexCoord);
    }
    else
    {
        output.TexCoord = input.TexCoord;
    }
    
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float4 color = g_useTexture ? g_texture[0].Sample(g_sampler, input.TexCoord) : g_baseColor;
    
    clip(color.a - 0.1f); // or clip(color.a - 0.01f) 로 조절 가능
    
    // Hover 상태이면 밝기 강화
    if (g_isHovered == 1)
    {
        color.rgb *= 1.1f; // 살짝 빛나는 느낌
    }
    else
    {
        color.rgb *= 0.6f;
    }
    if (g_fillAmount <= 1.0 && input.TexCoord.x > g_fillAmount)
        discard;

    
    return color;
    //return float4(1, 0, 0, 1); // 강제 빨간색
    
}