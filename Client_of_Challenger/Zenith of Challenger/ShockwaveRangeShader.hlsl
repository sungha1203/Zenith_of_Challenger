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
    float3 WorldPos : POSITION1;
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

float4 PSMain(PSInput input) : SV_Target
{
    float2 uv = input.TexCoord;
    float2 center = float2(0.5f, 0.5f);
    float dist = distance(uv, center);

// �簢�� �ȿ����� �������� �ִ� ������ ����
    float maxRadius = 0.5f;
    float currentRadius = g_fillAmount * maxRadius;

    float edge = 0.03f;

// ���� ��� ��
    float bg = smoothstep(maxRadius, maxRadius + 0.05f, dist);
    float3 bgColor = float3(1.0, 0.3, 0.3);
    float bgAlpha = (1.0 - bg) * 0.25f;

// ���� �߽� �� (�߽� �� �ٱ� ����, �ִ� �ݰ� ����)
    float coreMask = smoothstep(currentRadius - edge, currentRadius + edge, dist);
    float3 coreColor = float3(1.0, 0.0, 0.0);
    float coreAlpha = saturate(1.0 - coreMask) * 0.9f;

// ���� ���
    float3 finalColor = lerp(bgColor, coreColor, coreAlpha);
    float finalAlpha = bgAlpha + coreAlpha;

    return float4(finalColor, finalAlpha * g_baseColor.a);
}