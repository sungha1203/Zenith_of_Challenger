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

    float4 worldPosition = mul(float4(input.Position, 1.0f), g_worldMatrix);
    float4 viewPosition = mul(worldPosition, g_viewMatrix);
    output.Position = mul(viewPosition, g_projectionMatrix);

    output.WorldPos = worldPosition.xyz;
    output.Normal = normalize(mul(input.Normal, (float3x3) g_worldMatrix));
    output.TexCoord = input.TexCoord;

    return output;
}

float4 PSMain(PSInput input) : SV_Target
{
    float2 uv = input.TexCoord;

    // [1] �⺻ �ٴ� ��� ���� (�׻� ��)
    float3 baseColor = float3(1.0, 0.05, 0.05); // ���� ������
    float baseAlpha = 0.3f;

    // [2] ä������ ������ (fillAmount ��ŭ �Ʒ����� ���� ä����)
    float fill = step(uv.y, g_fillAmount); // �Ʒ����� ���� ä���� (UV.y <= g_fillAmount)

    float3 fillColor = float3(1.0, 0.2, 0.2); // ���� ������
    float fillAlpha = 0.6f;

    // [3] ���� ����� ����
    float3 finalColor = lerp(baseColor, fillColor, fill);
    float finalAlpha = lerp(baseAlpha, fillAlpha, fill);

    return float4(finalColor, finalAlpha);
}