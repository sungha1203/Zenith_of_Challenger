#include "DissolveDustEffectObject.h"
#include "Shader.h"
#include <random>

DissolveDustEffectObject::DissolveDustEffectObject(const ComPtr<ID3D12Device>& device)
    : GameObject(device)
{
    SetUseTexture(false);
    SetBaseColor({ 0.85f, 0.85f, 0.85f, 1.0f });
    SetScale({ 1.f, 1.f, 1.f });
}

void DissolveDustEffectObject::Spawn(const XMFLOAT3& center, int count)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, XM_2PI);
    std::uniform_real_distribution<float> radiusDist(0.0f, 50.5f);
    std::uniform_real_distribution<float> velYDist(0.3f, 6.5f); // 파티클마다 다른 속도
    std::uniform_real_distribution<float> scaleDist(0.002f, 0.05f);
    std::uniform_real_distribution<float> lifeDist(1.8f, 2.8f);
    std::uniform_real_distribution<float> yOffsetDist(0.0f, 15.0f); // y 위치 랜덤 오프셋

    m_particles.clear();

    for (int i = 0; i < count; ++i)
    {
        float angle = angleDist(gen);
        float radius = radiusDist(gen);
        float x = center.x + cosf(angle) * radius;
        float z = center.z + sinf(angle) * radius;
        float y = center.y + yOffsetDist(gen); // 중심보다 높거나 같음

        DustParticle p;
        p.position = { x, y, z };
        p.velocity = { 0.f, velYDist(gen), 0.f }; // y축 상승 속도 랜덤
        p.scale = scaleDist(gen);
        p.elapsed = 0.f;
        p.lifetime = lifeDist(gen);
        p.alpha = 1.f;
        XMStoreFloat4x4(&p.worldMatrix, XMMatrixIdentity());

        m_particles.push_back(p);
    }
}

void DissolveDustEffectObject::Update(float dt, const shared_ptr<Camera>& camera)
{
    XMFLOAT3 right = camera->GetU();
    XMFLOAT3 up = camera->GetV();

    for (auto& p : m_particles)
    {
        p.elapsed += dt;
        if (p.elapsed >= p.lifetime) continue;

        // 상승 이동 (파티클 개별 속도 적용)
        p.position.x += p.velocity.x * dt;
        p.position.y += p.velocity.y * dt;
        p.position.z += p.velocity.z * dt;

        // 알파 감소
        p.alpha = 1.0f - (p.elapsed / p.lifetime);

        // 빌보드 행렬 구성
        XMMATRIX world =
            XMMatrixScaling(p.scale, p.scale, 1.f) *
            XMMATRIX(
                XMLoadFloat3(&right),
                XMLoadFloat3(&up),
                XMVectorSet(0.f, 0.f, 1.f, 0.f),
                XMVectorSet(p.position.x, p.position.y, p.position.z, 1.f)
            );

        XMStoreFloat4x4(&p.worldMatrix, world);
    }

    // 사망한 파티클 제거
    m_particles.erase(
        std::remove_if(m_particles.begin(), m_particles.end(),
            [](const DustParticle& p) { return p.elapsed >= p.lifetime; }),
        m_particles.end());
}

void DissolveDustEffectObject::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    if (!m_shader || !m_mesh || m_particles.empty()) return;

    for (const auto& p : m_particles)
    {
        SetWorldMatrix(XMLoadFloat4x4(&p.worldMatrix));
        SetBaseColor({ 0.85f, 0.85f, 0.85f, p.alpha });
        UpdateShaderVariable(commandList, &p.worldMatrix);
        GameObject::Render(commandList);
    }
}