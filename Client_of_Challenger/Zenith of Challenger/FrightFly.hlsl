#pragma enable_d3d11_debug_symbols
#include "Shaders.hlsl"

struct VertexInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
    uint4 boneIndices : BLENDINDICES;
    float4 boneWeights : BLENDWEIGHT;
};

struct PixelInput
{
    float4 position : SV_POSITION;
    float3 worldPos : POSITION1;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
    float4 shadowPos : TEXCOORD1;
};

PixelInput VSMain(VertexInput input)
{
    PixelInput output;

    // �� ����Ʈ ����ȭ
    float totalWeight = input.boneWeights.x + input.boneWeights.y + input.boneWeights.z + input.boneWeights.w;
    if (totalWeight > 0.0001f)
        input.boneWeights /= totalWeight;
    else
        input.boneWeights = float4(1, 0, 0, 0);

    // ��ġ ��Ű��
    float4 pos = float4(input.position, 1.0f);
    float4 skinnedPos =
        mul(pos, g_MboneMatrices[input.boneIndices.x]) * input.boneWeights.x +
        mul(pos, g_MboneMatrices[input.boneIndices.y]) * input.boneWeights.y +
        mul(pos, g_MboneMatrices[input.boneIndices.z]) * input.boneWeights.z +
        mul(pos, g_MboneMatrices[input.boneIndices.w]) * input.boneWeights.w;

    float4 worldPos = mul(skinnedPos, g_worldMatrix);
    output.position = mul(worldPos, mul(g_viewMatrix, g_projectionMatrix));
    output.worldPos = worldPos.xyz;

    // ��� ��Ű��
    float3 n = normalize(input.normal);
    float3 skinnedNormal =
        mul((float3x3) g_MboneMatrices[input.boneIndices.x], n) * input.boneWeights.x +
        mul((float3x3) g_MboneMatrices[input.boneIndices.y], n) * input.boneWeights.y +
        mul((float3x3) g_MboneMatrices[input.boneIndices.z], n) * input.boneWeights.z +
        mul((float3x3) g_MboneMatrices[input.boneIndices.w], n) * input.boneWeights.w;

    output.normal = normalize(mul(skinnedNormal, (float3x3) g_worldMatrix));
    output.texcoord = input.texcoord;
    output.shadowPos = mul(worldPos, shadowMat);

    return output;
}

float4 PSMain(PixelInput input) : SV_Target
{
    float4 texColor = g_texture[0].Sample(g_sampler, input.texcoord);

    if (texColor.r + texColor.g + texColor.b < 0.01f)
        texColor.rgb = float3(1, 0, 1); // fallback color

    float3 normal = normalize(input.normal);
    float3 toEye = normalize(g_cameraPosition - input.worldPos);

    MaterialData matData;
    matData.fresnelR0 = float3(0.01f, 0.01f, 0.01f);
    matData.roughness = 0.5f;
    matData.ambient = float3(0.3f, 0.3f, 0.3f); // �ʿ信 ���� ����

    float shadow = ComputeShadowFactor(input.shadowPos.xyz);

    return Lighting(input.worldPos, normal, toEye, texColor, matData); //shadow
}
