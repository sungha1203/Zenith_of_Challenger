#include "Shaders.hlsl" // ���� ������� �� �ؽ�ó ���� ����

struct VSInput
{
    float3 pos : POSITION;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float depth : TEXCOORD0; // �� ���� z ���޿�
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;

    float4 world = mul(float4(input.pos, 1.0f), g_worldMatrix);
    float4 view = mul(world, g_shadowViewMatrix);
    float4 proj = mul(view, g_shadowProjMatrix);

    output.pos = mul(world, viewProjMat);;
    output.depth = view.z; // View space Z���� ����

    return output;
}

float PSMain(VSOutput input) : SV_Target
{
    float ndcDepth = input.pos.z / input.pos.w;
    return ndcDepth; 
}