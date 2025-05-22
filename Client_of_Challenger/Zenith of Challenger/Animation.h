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
    //DirectX::XMMATRIX globalInverseTransform = XMMatrixIdentity();
    //void SetGlobalInverseTransform(XMMATRIX m) { globalInverseTransform = m; }
    DirectX::XMMATRIX GetBoneTransform(const BoneAnimation& boneAnim, float time) const;
    //unordered_map<string, DirectX::XMMATRIX> GetBoneTransforms(float time) const;
    std::pair<std::vector<XMMATRIX>, std::unordered_map<std::string, int>> GetBoneTransforms(float time, unordered_map<std::string, int>boneNametoIndex, unordered_map<std::string, string>boneHierarchy, unordered_map<std::string, XMMATRIX>staticNodeTransforms) const;
    std::vector<XMMATRIX> AnimationClip::GetBoneTransforms(
        float time,
        const std::unordered_map<std::string, int>& boneNameToIndex,
        const std::unordered_map<std::string, std::string>& boneHierarchy,
        unordered_map<int, XMMATRIX> boneOffsets,
        const std::unordered_map<std::string, XMMATRIX>& initialGlobalTransforms) const;
};

XMMATRIX InterpolateKeyframes(const Keyframe& a, const Keyframe& b, float t, const std::string& currentBoneName, float currentTime);
vector<string> TopologicalSort(const unordered_map<string, string>& boneHierarchy);