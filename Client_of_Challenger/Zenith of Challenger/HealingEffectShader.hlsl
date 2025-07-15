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
    float3 WorldPos : WORLDPOS;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
};

PSInput VSMain(VSInput input)
{
    PSInput output;

    float4 worldPosition = mul(float4(input.Position, 1.0f), g_worldMatrix);
    float4 viewPosition = mul(worldPosition, g_viewMatrix);
    output.Position = mul(viewPosition, g_projectionMatrix);

    output.WorldPos = worldPosition.xyz;
    output.Normal = normalize(mul(input.Normal, (float3x3) g_worldMatrix));
    output.TexCoord = input.TexCoord;

    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float2 uv = input.TexCoord - float2(0.5f, 0.5f);
    float dist = length(uv) * 2.0f;

    float glow = saturate(1.0f - dist * dist);

    float pulse = 0.95f + 0.15f * sin(g_totalTime * 8.0f);
    float3 baseColor = float3(0.3f, 1.0f, 0.3f); // ÃÊ·Ïºû

    float3 color = baseColor * glow * pulse * 2.0f;

    float alpha = glow * pulse;

    return float4(color, alpha);
}
