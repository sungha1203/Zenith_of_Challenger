// FullScreenQuadShader.hlsl
#include "Shaders.hlsl"

struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

VSOutput VSMain(uint id : SV_VertexID)
{
    float2 pos[3] =
    {
        float2(-1.0f, -1.0f),
        float2(-1.0f, 3.0f),
        float2(3.0f, -1.0f)
    };

    float2 uv[3] =
    {
        float2(0.0f, 1.0f),
        float2(0.0f, -1.0f),
        float2(2.0f, 1.0f)
    };

    VSOutput output;
    output.pos = float4(pos[id], 0.0f, 1.0f);
    output.uv = uv[id];
    return output;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
    float depth = g_shadowMap.Sample(shadowSampler, input.uv).r;
    return float4(depth, depth, depth, 1.0f); // 0.0 °ËÁ¤ ~ 1.0 Èò»ö
}