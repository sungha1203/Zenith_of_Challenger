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
    float4 viewPos = mul(worldPos, g_viewMatrix);
    output.Position = mul(viewPos, g_projectionMatrix);

    output.TexCoord = input.TexCoord;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float4 color = g_useTexture ? g_texture[0].Sample(g_sampler, input.TexCoord) : g_baseColor;

    // 알파가 0에 가까우면 버림 (로고 배경 제거용)
    clip(color.a - 0.1f);

    return color;
}
