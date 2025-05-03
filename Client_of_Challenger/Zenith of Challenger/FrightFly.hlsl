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

//float4x4 ScaleMatrix(float4x4 m, float s)
//{
//	return float4x4(
//        m[0] * s,
//        m[1] * s,
//        m[2] * s,
//        m[3] * s
//    );
//}

PixelInput VSMain(VertexInput input)
{
	PixelInput output;

    float totalWeight = input.boneWeights.x + input.boneWeights.y + input.boneWeights.z + input.boneWeights.w;
    if (totalWeight > 0.0001f)
    {
        input.boneWeights /= totalWeight; // ����ȭ
    }
    else
    {
        input.boneWeights = float4(1, 0, 0, 0); // fallback: ù ��° ���� ���
    }
    
    float4 pos = float4(input.position, 1.0f);

// ������ �� ��Ŀ� ������ �����ְ�, weight�� ������
    float4 skinpos =
    mul(pos, g_MboneMatrices[input.boneIndices.x]) * input.boneWeights.x +
    mul(pos, g_MboneMatrices[input.boneIndices.y]) * input.boneWeights.y +
    mul(pos, g_MboneMatrices[input.boneIndices.z]) * input.boneWeights.z +
    mul(pos, g_MboneMatrices[input.boneIndices.w]) * input.boneWeights.w;
	
      // --- ���� �߰� ---
    //output.debugColor = float4(
    //input.boneIndices.x / 39.0f,
    //input.boneIndices.y / 39.0f,
    //input.boneIndices.z / 39.0f,
    //1.0f);
    // ------------------

    
    float4 worldPos = mul(skinpos, g_worldMatrix);
    output.position = mul(worldPos, mul(g_viewMatrix, g_projectionMatrix));
    output.worldPos = worldPos.xyz;
    
	 // ��� ��Ű�׵� ���ľ� ��
    float3 n = normalize(input.normal);

    float3 skinnedNormal =
        mul((float3x3) g_MboneMatrices[input.boneIndices.x], n) * input.boneWeights.x +
        mul((float3x3) g_MboneMatrices[input.boneIndices.y], n) * input.boneWeights.y +
        mul((float3x3) g_MboneMatrices[input.boneIndices.z], n) * input.boneWeights.z +
        mul((float3x3) g_MboneMatrices[input.boneIndices.w], n) * input.boneWeights.w;

    output.normal = normalize(mul(skinnedNormal, (float3x3) g_worldMatrix));
    output.texcoord = input.texcoord;
    
     // --- �߰�: ����׿� �÷� encode ---
    // ������� ����: boneIndices (0~39 ����) �� �÷��� �Ѹ���
   // output.debugColor = float4(
   //     input.boneIndices.x / 39.0f, // R: ù ��° �� �ε���
   //     input.boneIndices.y / 39.0f, // G: �� ��° �� �ε���
   //     input.boneIndices.z / 39.0f, // B: �� ��° �� �ε���
   //     1.0f // A: ����
   // );
    output.shadowPos = mul(worldPos, shadowMat);

	return output;
}

float4 PSMain(PixelInput input) : SV_Target
{
    float4 texColor = g_texture[0].Sample(g_sampler, input.texcoord);

    // �ʹ� ��ο�� fallback ����
    if (texColor.r + texColor.g + texColor.b < 0.01f)
        texColor.rgb = float3(1, 0, 1); // ����Ÿ
    
    float3 normal = normalize(input.normal);
    float3 toEye = normalize(g_cameraPosition - input.worldPos);
    
    MaterialData matData;
    matData.fresnelR0 = float3(0.01f, 0.01f, 0.01f);
    matData.roughness = 0.5f;
    matData.ambient = float3(0.8f, 0.8f, 0.8f);

    float shadow = ComputeShadowFactor(input.shadowPos.xyz);
	
	
    return Lighting(input.worldPos, normal, toEye, texColor, matData) * shadow;
    //float4 debugShadow = ComputeShadowFactor(input.worldPos);
    //return debugShadow;
    
}