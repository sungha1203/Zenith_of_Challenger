#include "Shaders.hlsl"

struct VS_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;
    float4 worldPos = mul(float4(input.position, 1.0f), g_worldMatrix);
    float4 viewPos = mul(worldPos, g_viewMatrix);
    output.position = mul(viewPos, g_projectionMatrix);
    output.color = g_baseColor;
    return output;
}

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    return input.color;
}
