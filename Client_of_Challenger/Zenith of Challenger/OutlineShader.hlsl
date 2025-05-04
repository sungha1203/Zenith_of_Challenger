#include "Shaders.hlsl"

//입력 구조 - SkinnedVertex 기준
struct VS_IN
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    uint4 indices : BLENDINDICES;
    float4 weights : BLENDWEIGHT;
};

//VS → PS 구조
struct VS_OUT
{
    float4 pos : SV_POSITION;
};

//Vertex Shader - 애니메이션 스키닝 + 외곽선 offset
VS_OUT VSMain(VS_IN input)
{
    VS_OUT output;

    float4 skinnedPos = float4(0, 0, 0, 0);
    float3 skinnedNormal = float3(0, 0, 0);

    // 최대 4개의 본 가중치 적용
    for (int i = 0; i < 4; ++i)
    {
        uint boneIndex = input.indices[i];
        float weight = input.weights[i];

        float4x4 boneMatrix = g_boneMatrices[boneIndex];

        skinnedPos += mul(float4(input.pos, 1.0f), boneMatrix) * weight;
        skinnedNormal += mul(float4(input.normal, 0.0f), boneMatrix).xyz * weight;
    }

    // 외곽선 offset 적용
    float3 offset = normalize(skinnedNormal) * 0.15f; // 외곽선 두께
    float4 worldPos = mul(skinnedPos + float4(offset, 0.0f), g_worldMatrix);
    float4 viewPos = mul(worldPos, g_viewMatrix);
    output.pos = mul(viewPos, g_projectionMatrix);

    return output;
}

// Pixel Shader - 외곽선 색상 출력
float4 PSMain(VS_OUT input) : SV_Target
{
    // 원하는 외곽선 색으로 변경 가능
    return float4(0.0f, 0.0f, 0.0f, 1.0f); // 검정 외곽선
}