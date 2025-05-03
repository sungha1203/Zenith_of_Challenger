#ifndef __SHADERS_HLSL__
#define __SHADERS_HLSL__

// -----------------------------
// 상수 버퍼 정의
// -----------------------------

// GameObject: b0, space0
cbuffer GameObject : register(b0)
{
    matrix g_worldMatrix : packoffset(c0); // c0~c3 (16 x 4 = 64 byte)
    float4 g_baseColor : packoffset(c4); // c4      (16 byte)
    int g_useTexture : packoffset(c5.x); // c5.x (4 byte)
    int g_textureIndex : packoffset(c5.y); // c5.y (4 byte)
    int g_isHovered : packoffset(c5.z); // c5.z (4 byte)
    float g_fillAmount : packoffset(c5.w);
    //float padding : packoffset(c5.w); // c5.w (4 byte) → 총 16 byte 정렬        b0: 총 96
};

// Camera: b1, space0
cbuffer Camera : register(b1)
{
    matrix g_viewMatrix : packoffset(c0); // c0 ~ c3
    matrix g_projectionMatrix : packoffset(c4); // c4 ~ c7
    float3 g_cameraPosition : packoffset(c8); // c8
};

// Material: b2, space0
cbuffer Material : register(b2)
{
    float4 g_materialColor;
    int g_useLighting;
    float3 padding2;
};

// Light: b3, space0
cbuffer Light : register(b3)
{
    float3 g_lightDirection;
    float g_lightIntensity;
};

// -----------------------------
// Lighting 연산 함수
// -----------------------------
#include "Lighting.hlsl"

// -----------------------------
// 텍스처 & 샘플러
// -----------------------------

// 텍스처 슬롯
TextureCube g_textureCube : register(t0);
Texture2D g_texture[13] : register(t1); // t1 ~ t10 → space0

// InstanceData: t0, space1
struct InstanceData
{
    float4x4 worldMatrix;
    uint textureIndex;
    uint materialIndex;
};
StructuredBuffer<InstanceData> g_instanceData : register(t0, space1);

// BoneMatrix: t2, space0
StructuredBuffer<float4x4> g_boneMatrices : register(t14);
StructuredBuffer<float4x4> g_MboneMatrices : register(t15);

// 샘플러
SamplerState g_sampler : register(s0);

#endif // __SHADERS_HLSL__
