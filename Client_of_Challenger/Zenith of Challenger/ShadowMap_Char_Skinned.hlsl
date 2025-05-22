#include "Shaders.hlsl"

struct VSInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float4 boneWeights : BLENDWEIGHT;
    uint4 boneIndices : BLENDINDICES;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float4 shadowCoord : TEXCOORD0;
};

float4x4 ScaleMatrix(float4x4 m, float s)
{
    return float4x4(
        m[0] * s,
        m[1] * s,
        m[2] * s,
        m[3] * s
    );
}

VSOutput VSMain(VSInput input)
{
    VSOutput output;

    float4 pos = float4(input.pos, 1.0f);

    // boneWeights 정규화
    float4 weights = input.boneWeights;
    float weightSum = dot(weights, float4(1, 1, 1, 1));
    if (weightSum > 0.0001f)
        weights /= weightSum;
    else
        weights = float4(1, 0, 0, 0);

    // 스키닝 행렬 가중합
    float4x4 skinMatrix =
        ScaleMatrix(g_boneMatrices[input.boneIndices.x], weights.x) +
        ScaleMatrix(g_boneMatrices[input.boneIndices.y], weights.y) +
        ScaleMatrix(g_boneMatrices[input.boneIndices.z], weights.z) +
        ScaleMatrix(g_boneMatrices[input.boneIndices.w], weights.w);

    float4 skinned = mul(pos, skinMatrix);
    float4 world = mul(skinned, g_worldMatrix);
    float4 lightViewProj = mul(world, g_shadowViewMatrix);
    lightViewProj = mul(lightViewProj, g_shadowProjMatrix);

    output.pos = lightViewProj;
    output.shadowCoord = lightViewProj; // NDC로 바로 넘김

    return output;
}

float PSMain(VSOutput input) : SV_Target
{
    // NDC z 깊이 저장
    float ndcDepth = input.shadowCoord.z / input.shadowCoord.w;
    return ndcDepth;
}
