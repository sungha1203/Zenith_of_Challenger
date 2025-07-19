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
    // �߽� �Ÿ� ���
    float2 uv = input.TexCoord - float2(0.5f, 0.5f);
    float dist = length(uv) * 2.0f;

    // �߽��� ��� �ܰ��� ����� glow ȿ��
    float glow = saturate(1.0f - dist * dist); // �ε巴�� ����

    // �ð��� ���� �޽� ȿ�� (��¦ �����̵���)
    float pulse = 0.95f + 0.15f * sin(g_totalTime * 8.0f);

    // g_baseColor ��� ���� ���
    float3 color = g_baseColor.rgb * glow * pulse * 2.0f;

    // ���ĵ� glow ��� + pulse ���� + g_baseColor.a �ݿ�
    float alpha = glow * pulse * g_baseColor.a;

    // �ʹ� �帰 �ȼ� ����
    clip(alpha - 0.05f);

    return float4(color, alpha);
}
