#pragma once
// LineShader.hlsl

#include"Shaders.hlsl"

struct VSInput
{
    float3 position : POSITION;
};

struct PSInput
{
    float4 position : SV_POSITION;
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    float4 worldPos = mul(float4(input.position, 1.0f), g_worldMatrix);
    float4 viewPos = mul(worldPos, g_viewMatrix);
    output.position = mul(viewPos, g_projectionMatrix);
    return output;
}

float4 PSMain(PSInput input) : SV_Target
{
    return float4(1, 0, 0, 1); // »¡°£ ¼±
}
