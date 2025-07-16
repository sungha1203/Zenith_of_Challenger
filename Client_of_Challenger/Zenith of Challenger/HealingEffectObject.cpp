#include "HealingEffectObject.h"

HealingEffectObject::HealingEffectObject(const ComPtr<ID3D12Device>& device)
    : GameObject(device)
{
	m_scale = { 1.0f, 1.0f, 1.0f };
}

void HealingEffectObject::Update(FLOAT timeElapsed, const std::shared_ptr<Camera>& camera)
{
    XMFLOAT3 basePos = m_followTarget ? m_followTarget->GetPosition() : GetPosition();

    for (auto& p : m_particles)
    {
        p.elapsed += timeElapsed;

        // �̵�: �߽ɿ��� + ���� ����
        p.position.x += p.velocity.x * timeElapsed;
        p.position.y += p.velocity.y * timeElapsed;
        p.position.z += p.velocity.z * timeElapsed;

        // ���� ����
        p.alpha = 1.0f - (p.elapsed / p.lifetime);

        // ������
        if (camera)
        {
            XMFLOAT3 right = camera->GetU();
            XMFLOAT3 up = camera->GetV();

            XMMATRIX world =
                XMMatrixScaling(p.scale, p.scale, 1.f) *
                XMMATRIX(
                    XMLoadFloat3(&right),
                    XMLoadFloat3(&up),
                    XMVectorSet(0.f, 0.f, 1.f, 0.f),
                    XMVectorSet(p.position.x, p.position.y - 5.f, p.position.z, 1.f)
                );

            XMStoreFloat4x4(&p.worldMatrix, world);
        }
    }

    // ����
    m_particles.erase(
        std::remove_if(m_particles.begin(), m_particles.end(),
            [](const HealingParticle& p) {
                return p.elapsed >= p.lifetime;
            }),
        m_particles.end()
    );
}

void HealingEffectObject::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    for (const auto& p : m_particles)
    {
        m_worldMatrix = p.worldMatrix;
        m_baseColor = XMFLOAT4(0.3f, 1.f, 0.3f, p.alpha);
        GameObject::Render(commandList);
    }
}

void HealingEffectObject::InitializeParticles()
{
    m_particles.clear();
    for (int i = 0; i < PARTICLE_COUNT; ++i)
    {
        HealingParticle p;
        p.position = GetPosition();

        // ���� ���� (XZ ���)
        float angle = XM_2PI * (rand() % 1000) / 1000.0f;
        float speed = 10.0f + static_cast<float>(rand() % 50) / 100.0f;
        p.velocity = XMFLOAT3(cos(angle) * speed, 2.0f, sin(angle) * speed);
        p.scale = 0.005f + static_cast<float>(rand() % 100) / 10000.0f;
        p.alpha = 1.0f - powf(p.elapsed / p.lifetime, 1.2f);
        p.lifetime = 1.0f;
        p.elapsed = 0.0f;

        m_particles.push_back(p);
    }
}
