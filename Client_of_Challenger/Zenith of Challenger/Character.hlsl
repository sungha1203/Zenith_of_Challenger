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

float4x4 ScaleMatrix(float4x4 m, float s)
{
    return float4x4(
        m[0] * s,
        m[1] * s,
        m[2] * s,
        m[3] * s
    );
}

PixelInput VSMain(VertexInput input)
{
    PixelInput output;
    // ��Ű�� ��� ���
    float4x4 skinMatrix =
        ScaleMatrix(g_boneMatrices[input.boneIndices.x], input.boneWeights.x) +
        ScaleMatrix(g_boneMatrices[input.boneIndices.y], input.boneWeights.y) +
        ScaleMatrix(g_boneMatrices[input.boneIndices.z], input.boneWeights.z) +
        ScaleMatrix(g_boneMatrices[input.boneIndices.w], input.boneWeights.w);

    // ��ġ ��ȯ
    float4 localPos = mul(float4(input.position, 1.0f), skinMatrix);
    float4 worldPos = mul(localPos, g_worldMatrix);
    float4 viewProjPos = mul(worldPos, mul(g_viewMatrix, g_projectionMatrix));

    output.position = viewProjPos;
    output.worldPos = worldPos.xyz;

    // ��� ��Ű�� (��Ȯ�� ���)
    float3 skinnedNormal =
        mul((float3x3) g_boneMatrices[input.boneIndices.x], input.normal) * input.boneWeights.x +
        mul((float3x3) g_boneMatrices[input.boneIndices.y], input.normal) * input.boneWeights.y +
        mul((float3x3) g_boneMatrices[input.boneIndices.z], input.normal) * input.boneWeights.z +
        mul((float3x3) g_boneMatrices[input.boneIndices.w], input.normal) * input.boneWeights.w;

    float3 worldNormal = mul(skinnedNormal, (float3x3) g_worldMatrix);
    output.normal = normalize(worldNormal);

    output.texcoord = input.texcoord;
    output.shadowPos = mul(worldPos, shadowMat);

    return output;
}

float4 PSMain(PixelInput input) : SV_Target
{
    // �ؽ�ó ���ø�
    float4 texColor = g_texture[0].Sample(g_sampler, input.texcoord);

    // �ʹ� ��ο�� fallback (����Ÿ)
    if (texColor.r + texColor.g + texColor.b < 0.01f)
        texColor.rgb = float3(1, 0, 1);

    // ���� ���
    float3 normal = normalize(input.normal);
    float3 toEye = normalize(g_cameraPosition - input.worldPos);

    // ��Ƽ���� ������ ����
    MaterialData matData;
    matData.fresnelR0 = float3(0.01f, 0.01f, 0.01f);
    matData.roughness = 0.5f;
    matData.ambient = float3(0.5f, 0.5f, 0.5f);

    // �׸��� ��� ���
    float shadow = ComputeShadowFactor(input.shadowPos.xyz);

    // ���� ���� �� �׸��� ����
    return Lighting(input.worldPos, normal, toEye, texColor, matData) * 0.7f; //* shadow

    // ����׿� �׸��� Ȯ��
    // return ComputeShadowFactor(input.worldPos);
}
