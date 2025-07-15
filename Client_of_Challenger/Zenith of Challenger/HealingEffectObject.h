#pragma once
#include "Object.h"
#include "Camera.h"

class HealingEffectObject : public GameObject
{
public:
    HealingEffectObject(const ComPtr<ID3D12Device>& device);
    ~HealingEffectObject() override = default;

    void Update(float timeElapsed, const std::shared_ptr<Camera>& camera);

    void SetLifetime(float sec) { m_lifetime = sec; }
    float GetElapsed() const { return m_elapsed; }
    float GetLifetime() const { return m_lifetime; }

    void SetFollowTarget(const std::shared_ptr<GameObject>& target) { m_followTarget = target; }

private:
    float m_elapsed = 0.f;
    float m_lifetime = 1.0f;

    std::shared_ptr<GameObject> m_followTarget;
};
