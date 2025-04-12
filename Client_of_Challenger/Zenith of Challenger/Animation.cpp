// animation.cpp
#include "animation.h"
#include <DirectXMath.h>
using namespace DirectX;

XMMATRIX InterpolateKeyframes(const Keyframe& a, const Keyframe& b, float t)
{
    XMVECTOR posA = XMLoadFloat3(&a.position);
    XMVECTOR posB = XMLoadFloat3(&b.position);
    XMVECTOR scaleA = XMLoadFloat3(&a.scale);
    XMVECTOR scaleB = XMLoadFloat3(&b.scale);
    XMVECTOR rotA = XMLoadFloat4(&a.rotation);
    XMVECTOR rotB = XMLoadFloat4(&b.rotation);

    XMVECTOR pos = XMVectorLerp(posA, posB, t);
    XMVECTOR scale = XMVectorLerp(scaleA, scaleB, t);
    XMVECTOR rot = XMQuaternionSlerp(rotA, rotB, t);

    XMMATRIX T = XMMatrixTranslationFromVector(pos);
    XMMATRIX R = XMMatrixRotationQuaternion(rot);
    XMMATRIX S = XMMatrixScalingFromVector(scale);

    return S * R * T;
}

XMMATRIX AnimationClip::GetBoneTransform(const BoneAnimation& boneAnim, float time) const
{
    const auto& keys = boneAnim.keyframes;
    if (keys.empty()) return XMMatrixIdentity();
    if (keys.size() == 1) return InterpolateKeyframes(keys[0], keys[0], 0);

    Keyframe a = keys[0];
    Keyframe b = keys[0];

    for (size_t i = 0; i < keys.size() - 1; ++i)
    {
        if (time < keys[i + 1].time)
        {
            a = keys[i];
            b = keys[i + 1];
            break;
        }
    }

    float lerpFactor = (time - a.time) / (b.time - a.time);
    return InterpolateKeyframes(a, b, lerpFactor);
}

unordered_map<string, XMMATRIX> AnimationClip::GetBoneTransforms(float time) const
{
    unordered_map<string, XMMATRIX> transforms;

    for (const auto& [boneName, boneAnim] : boneAnimations)
    {
        transforms[boneName] = GetBoneTransform(boneAnim, time);
    }

    return transforms;
}