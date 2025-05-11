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
    float depth : TEXCOORD0;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;

    float4 pos = float4(input.pos, 1.0f);

    // boneWeights 정규화
    float weightSum = input.boneWeights.x + input.boneWeights.y + input.boneWeights.z + input.boneWeights.w;
    float4 weights = input.boneWeights;
    if (weightSum > 0.0001f)
        weights /= weightSum;
    else
        weights = float4(1, 0, 0, 0); // fallback: 첫 번째 본만 사용

    // 가중합 스키닝 포지션 계산
    float4 skinned =
        mul(pos, g_boneMatrices[input.boneIndices.x]) * weights.x +
        mul(pos, g_boneMatrices[input.boneIndices.y]) * weights.y +
        mul(pos, g_boneMatrices[input.boneIndices.z]) * weights.z +
        mul(pos, g_boneMatrices[input.boneIndices.w]) * weights.w;

    float4 world = mul(skinned, g_worldMatrix);
    float4 view = mul(world, g_shadowViewMatrix);
    float4 proj = mul(view, g_shadowProjMatrix);

    output.pos = proj;
    output.depth = view.z;

    return output;
}

float PSMain(VSOutput input) : SV_Target
{
    float ndcDepth = input.pos.z / input.pos.w;
    return ndcDepth;
}
