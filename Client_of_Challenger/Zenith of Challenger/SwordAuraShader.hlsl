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
    // 중심 밝기 강조용: 중심에서 멀어질수록 어둡게
    float2 center = float2(0.5f, 0.5f);
    float2 uv = input.TexCoord;
    float dist = distance(uv, center);

    // 중앙 밝기 강조
    float brightness = saturate(1.0f - dist * 2.0f); // 외곽으로 갈수록 어두워짐

    // 흐르는 라인 느낌 (Y 기준으로 wave)
    float wave = sin((uv.y + g_totalTime * 2.0f) * 20.0f) * 0.1f;

    // 파형을 알파에 반영해서 반짝이는 라인 느낌
    float alpha = saturate(brightness + wave + 0.3f);

    // 컬러는 고정 + 밝기 강조
    float3 baseColor = g_baseColor.rgb * (1.0f + brightness * 1.2f);

    return float4(baseColor, alpha);
}
