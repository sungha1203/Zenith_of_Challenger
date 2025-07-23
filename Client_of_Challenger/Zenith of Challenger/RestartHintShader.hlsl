#ifndef __RESTART_HINT_SHADER_HLSL__
#define __RESTART_HINT_SHADER_HLSL__

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

// �ܼ� ���� �� UV ���޿�
PSInput VSMain(VSInput input)
{
    PSInput output;

    float4 worldPos = mul(float4(input.Position, 1.0f), g_worldMatrix);

    float4x4 transformLikeStartScene =
    {
        1.8, 0.0, 0.0, 0.0,
        0.0, 2.4, 0.0, 0.0,
        0.0, 0.0, 1.0, 1.0,
        0.0, 0.0, -0.1, 0.0
    };

    output.Position = mul(worldPos, transformLikeStartScene);

    output.TexCoord = (g_useCustomUV == 1)
        ? lerp(g_customUV.xy, g_customUV.zw, input.TexCoord)
        : input.TexCoord;

    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    // �ؽ�ó �Ǵ� �ܻ�
    float4 color = g_useTexture ? g_texture[0].Sample(g_sampler, input.TexCoord) : g_baseColor;

    // ���� �� ������: sin ��� 0~1 �ݺ�
    float flicker = abs(sin(g_totalTime * 2.0f));
    color.a *= flicker;

    // ���� Ŭ����
    clip(color.a - 0.1f);

    // Hover ������ �� ��� ����
    if (g_isHovered == 1)
        color.rgb *= 1.1f;
    else
        color.rgb *= 0.6f;

    // fillAmount ���� Ŭ����
    if (g_fillAmount <= 1.0 && input.TexCoord.x > g_fillAmount)
        discard;

    return color;
}

#endif // __RESTART_HINT_SHADER_HLSL__
