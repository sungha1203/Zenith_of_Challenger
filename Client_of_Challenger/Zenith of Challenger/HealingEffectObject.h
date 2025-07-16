#pragma once
#include "Object.h"
#include "Camera.h"

struct HealingParticle {
    XMFLOAT3 position;         // 현재 위치
    XMFLOAT3 velocity;         // 이동 방향 (랜덤)
    float scale = 1.0f;        // 크기
    float alpha = 1.0f;        // 투명도
    float elapsed = 0.0f;
    float lifetime = 1.0f;
    XMFLOAT4X4 worldMatrix;
};

class HealingEffectObject : public GameObject
{
public:
    HealingEffectObject(const ComPtr<ID3D12Device>& device);
    ~HealingEffectObject() override = default;

    void Update(float timeElapsed, const shared_ptr<Camera>& camera);
    virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) override;

    void SetLifetime(float sec) { m_lifetime = sec; }
    float GetElapsed() const { return m_elapsed; }
    float GetLifetime() const { return m_lifetime; }

    void SetFollowTarget(const shared_ptr<GameObject>& target) { m_followTarget = target; }
    void InitializeParticles();

private:
    float m_elapsed = 0.f;
    float m_lifetime = 1.0f;


    vector<HealingParticle> m_particles;
    const int PARTICLE_COUNT = 20;

    shared_ptr<GameObject> m_followTarget;
};
