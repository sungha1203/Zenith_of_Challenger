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

// Vertex Shader
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
    float3 baseColor = float3(0.3f, 0.5f, 1.0f);

    float2 centeredUV = input.TexCoord - float2(0.5f, 0.5f);
    float dist = length(centeredUV) * 2.0f;
    float glow = saturate(1.0f - dist * dist); // 중심 밝음

    float pulse = 0.95f + 0.1f * sin(g_totalTime * 6.0f);

    float brightness = 2.0f;
    float3 color = baseColor * glow * pulse * brightness;

    color = min(color, float3(1.0f, 0.7f, 1.0f)); // 최대치를 보라색 톤으로 유지

    float alpha = lerp(0.6f, 1.0f, glow);

    return float4(color, alpha);
}

