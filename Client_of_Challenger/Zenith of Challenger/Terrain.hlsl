#include "Shaders.hlsl"

struct DETAIL_VERTEX_INPUT
{
    float3 position : POSITION;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
};

struct DETAIL_PIXEL_INPUT
{
    float4 position : SV_POSITION;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
    float4 shadowPos : TEXCOORD2;
};

DETAIL_PIXEL_INPUT DETAIL_VERTEX(DETAIL_VERTEX_INPUT input)
{
    DETAIL_PIXEL_INPUT output;

    float4 worldPos = mul(float4(input.position, 1.0f), g_worldMatrix);

    // ��-�������� ��ȯ (ȭ�� ��ġ)
    output.position = mul(worldPos, g_viewMatrix);
    output.position = mul(output.position, g_projectionMatrix);

    // �׸��� ��ǥ�� shadowMat���� �� ���� ��ȯ (View * Proj * Bias)
    output.shadowPos = mul(worldPos, shadowMat);

    // �ؽ�ó ��ǥ ����
    output.uv0 = input.uv0;
    output.uv1 = input.uv1;

    return output;
}

float4 DETAIL_PIXEL(DETAIL_PIXEL_INPUT input) : SV_TARGET
{
    // �ؽ�ó ����
    float4 texColor = lerp(
        g_texture[0].Sample(g_sampler, input.uv0),
        g_texture[1].Sample(g_sampler, input.uv1),
        0.5f
    );

    // ComputeShadowFactor�� ���ο��� .xyz/w, bias, 0~1 ��ȯ ��� ����
    float shadow = ComputeShadowFactor(input.shadowPos.xyz);

    return texColor * shadow;
}
