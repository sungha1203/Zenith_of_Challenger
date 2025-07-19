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

    float4 worldPos = mul(float4(input.Position, 1.0f), g_worldMatrix);
    float4 viewPos = mul(worldPos, g_viewMatrix);
    output.Position = mul(viewPos, g_projectionMatrix);

    output.WorldPos = worldPos.xyz;
    output.Normal = normalize(mul(input.Normal, (float3x3) g_worldMatrix));
    output.TexCoord = input.TexCoord;

    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    // 중심 거리 계산
    float2 uv = input.TexCoord - float2(0.5f, 0.5f);
    float dist = length(uv) * 2.0f;

    // 중심이 밝고 외곽이 희미한 glow 효과
    float glow = saturate(1.0f - dist * dist); // 부드럽게 감쇠

    // 시간에 따라 펄스 효과 (살짝 깜빡이도록)
    float pulse = 0.95f + 0.15f * sin(g_totalTime * 8.0f);

    // g_baseColor 기반 색상 사용
    float3 color = g_baseColor.rgb * glow * pulse * 2.0f;

    // 알파도 glow 기반 + pulse 조정 + g_baseColor.a 반영
    float alpha = glow * pulse * g_baseColor.a;

    // 너무 흐린 픽셀 제거
    clip(alpha - 0.05f);

    return float4(color, alpha);
}
