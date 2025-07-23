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
    float4 worldPos = mul(float4(input.Position, 1.0f), g_worldMatrix);
    float4 viewPos = mul(worldPos, g_viewMatrix);
    output.Position = mul(viewPos, g_projectionMatrix);
    output.WorldPos = worldPos.xyz;
    output.Normal = normalize(mul(input.Normal, (float3x3) g_worldMatrix));
    output.TexCoord = input.TexCoord;
    return output;
}

// Pixel Shader
float4 PSMain(PSInput input) : SV_Target
{
    // �߽� ��� ������: �߽ɿ��� �־������� ��Ӱ�
    float2 center = float2(0.5f, 0.5f);
    float2 uv = input.TexCoord;
    float dist = distance(uv, center);

    // �߾� ��� ����
    float brightness = saturate(1.0f - dist * 2.0f); // �ܰ����� ������ ��ο���

    // �帣�� ���� ���� (Y �������� wave)
    float wave = sin((uv.y + g_totalTime * 2.0f) * 20.0f) * 0.1f;

    // ������ ���Ŀ� �ݿ��ؼ� ��¦�̴� ���� ����
    float alpha = saturate(brightness + wave + 0.3f);

    // �÷��� ���� + ��� ����
    float3 baseColor = g_baseColor.rgb * (1.0f + brightness * 1.2f);

    return float4(baseColor, alpha);
}
