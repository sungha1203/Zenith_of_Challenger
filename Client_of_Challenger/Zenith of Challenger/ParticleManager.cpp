#include "ParticleManager.h"

void ParticleManager::Initialize(const ComPtr<ID3D12Device>& device, size_t maxParticles, shared_ptr<MeshBase> mesh, shared_ptr<Shader> shader)
{
    m_particles.reserve(maxParticles);
    for (size_t i = 0; i < maxParticles; ++i)
    {
        auto particle = make_shared<ParticleEffect>();
        particle->Initialize(device, mesh, shader);
        particle->Deactivate();
        m_particles.push_back(particle);
    }
}

void ParticleManager::SpawnParticle(const XMFLOAT3& position)
{
    for (auto& particle : m_particles)
    {
        if (!particle->IsAlive())
        {
            particle->Activate(position);
            return;
        }
    }
    // 다 사용중이면 무시
}

void ParticleManager::Update(float deltaTime)
{
    for (int i = 0; i < m_particles.size(); ++i)
    {
        m_particles[i]->Update(deltaTime);

        // 디버그 출력: 개별 파티클 위치 확인
        char msg[128];
        XMFLOAT3 pos = m_particles[i]->GetPosition();
        sprintf_s(msg, "Particle[%d] Position: %.2f, %.2f, %.2f\n", i, pos.x, pos.y, pos.z);
        //OutputDebugStringA(msg);
    }
}

void ParticleManager::Render(const ComPtr<ID3D12GraphicsCommandList>& cmdList) const
{
    for (const auto& particle : m_particles)
    {
        particle->Render(cmdList);
    }
}
