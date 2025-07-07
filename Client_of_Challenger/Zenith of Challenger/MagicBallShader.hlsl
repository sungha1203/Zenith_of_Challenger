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
    // 보라빛 마법 에너지 색
    float3 baseColor = float3(1.0f, 0.3f, 1.0f);

    // 중심 기준 Glow 계산
    float2 centeredUV = input.TexCoord - float2(0.5f, 0.5f);
    float dist = length(centeredUV) * 2.0f;

    // Glow는 중심에서 가장 밝고 바깥은 점점 어두워짐
    float glow = saturate(1.0f - dist * dist);

    // 펄스: 0.8 ~ 1.0 범위로 깜빡임 (어두워지지 않게)
    float pulse = 0.9f + 0.1f * sin(g_totalTime * 6.0f);

    // 최종 glow 강도 (최소 밝기 유지)
    glow = lerp(0.6f, 1.0f, glow) * pulse;

    // 빛나게 하기 위해 감마나 강도를 높임
    float brightness = 2.5f;
    float3 color = baseColor * glow * brightness;

    // 밝고 야광 같은 마법 볼 색상 반환 (alpha도 glow 기반)
    return float4(color, glow);
}
