#pragma once
#include "Object.h"
#include "Camera.h"

struct DustParticle
{
    XMFLOAT3 position;
    XMFLOAT3 velocity;
    float scale;
    float elapsed;
    float lifetime;
    float alpha;
    XMFLOAT4X4 worldMatrix;
};

class DissolveDustEffectObject : public GameObject
{
public:
    DissolveDustEffectObject(const ComPtr<ID3D12Device>& device);
    ~DissolveDustEffectObject() override = default;

    void Update(float timeElapsed, const shared_ptr<Camera>& camera);
    void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) override;

    void Spawn(const XMFLOAT3& origin, int particleCount = 30);
    bool IsFinished() const { return m_particles.empty(); }

private:
    vector<DustParticle> m_particles;
};
