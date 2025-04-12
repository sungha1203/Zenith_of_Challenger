#include "Shaders.hlsl"

struct SKYBOX_VERTEX_INPUT
{
    float3 position : POSITION;
};

struct SKYBOX_PIXEL_INPUT
{
    float4 position : SV_POSITION;
    float3 lookup : LOOKUP;
};

SKYBOX_PIXEL_INPUT SKYBOX_VERTEX(SKYBOX_VERTEX_INPUT input)
{
    SKYBOX_PIXEL_INPUT output;
    output.position = mul(float4(input.position, 1.0f), g_worldMatrix);
    output.position = mul(output.position, g_viewMatrix);
    output.position = mul(output.position, g_projectionMatrix).xyww;
    output.lookup = input.position;

    return output;
}

float4 SKYBOX_PIXEL(SKYBOX_PIXEL_INPUT input) : SV_TARGET
{
    return g_textureCube.Sample(g_sampler, input.lookup);
    //return float4(1, 0, 1, 1); // 매지엔타 → 잘 나오면 렌더링은 OK, 텍스처 샘플링 문제
    
}