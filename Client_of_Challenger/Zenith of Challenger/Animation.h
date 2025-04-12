#pragma once
#include"stdafx.h"

struct Keyframe
{
    float time;
    XMFLOAT3 position;
    XMFLOAT4 rotation;
    XMFLOAT3 scale;
};

struct BoneAnimation
{
    string boneName;
    vector<Keyframe> keyframes;
};

struct AnimationClip
{
    string name;
    float duration;         // 전체 재생 시간 (ticks)
    float ticksPerSecond;   // 초당 틱 수

    unordered_map<string, BoneAnimation> boneAnimations; // boneName → 애니메이션 정보

    DirectX::XMMATRIX GetBoneTransform(const BoneAnimation& boneAnim, float time) const;
    unordered_map<string, DirectX::XMMATRIX> GetBoneTransforms(float time) const;
};

DirectX::XMMATRIX InterpolateKeyframes(const Keyframe& a, const Keyframe& b, float t);