#include "Shaders.hlsl"

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float4 weights : BLENDWEIGHT;
    uint4 indices : BLENDINDICES;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float4 shadowCoord : TEXCOORD0;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;

    float4 pos = float4(input.position, 1.0f);

    // === 정확한 스키닝 (행렬 * 벡터, 곱해서 누적) ===
    float4 skinned =
        mul(pos, g_boneMatrices[input.indices.x]) * input.weights.x +
        mul(pos, g_boneMatrices[input.indices.y]) * input.weights.y +
        mul(pos, g_boneMatrices[input.indices.z]) * input.weights.z +
        mul(pos, g_boneMatrices[input.indices.w]) * input.weights.w;

    // 월드 좌표 변환
    float4 world = mul(skinned, g_worldMatrix);

    // 그림자 좌표 계산
    float4 lightViewProj = mul(world, g_shadowViewMatrix);
    lightViewProj = mul(lightViewProj, g_shadowProjMatrix);

    output.pos = lightViewProj;
    output.shadowCoord = lightViewProj;

    return output;
}

float PSMain(VSOutput input) : SV_Target
{
    // NDC 좌표 기준 깊이값 출력
    float ndcDepth = input.shadowCoord.z / input.shadowCoord.w;
    return ndcDepth;
}
