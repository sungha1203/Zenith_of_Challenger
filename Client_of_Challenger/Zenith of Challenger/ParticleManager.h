#pragma once
#include "ParticleEffect.h"

class ParticleManager
{
public:
    void Initialize(const ComPtr<ID3D12Device>& device, size_t maxParticles, shared_ptr<MeshBase> mesh, shared_ptr<Shader> shader);
    void SpawnParticle(const XMFLOAT3& position);
    void Update(float deltaTime);
    void Render(const ComPtr<ID3D12GraphicsCommandList>& cmdList) const;

private:
    vector<shared_ptr<ParticleEffect>> m_particles;
};
