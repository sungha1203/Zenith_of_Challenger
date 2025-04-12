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
};

DETAIL_PIXEL_INPUT DETAIL_VERTEX(DETAIL_VERTEX_INPUT input)
{
    DETAIL_PIXEL_INPUT output;
    output.position = mul(float4(input.position, 1.0f), g_worldMatrix);
    output.position = mul(output.position, g_viewMatrix);
    output.position = mul(output.position, g_projectionMatrix);
    output.uv0 = input.uv0;
    output.uv1 = input.uv1;

    return output;
}

float4 DETAIL_PIXEL(DETAIL_PIXEL_INPUT input) : SV_TARGET
{
    //return g_texture[0].Sample(g_sampler, input.uv0); // 하나의 텍스처만 샘플링
 
    return lerp(g_texture[0].Sample(g_sampler, input.uv0),
    g_texture[1].Sample(g_sampler, input.uv1), 0.5f);
}
