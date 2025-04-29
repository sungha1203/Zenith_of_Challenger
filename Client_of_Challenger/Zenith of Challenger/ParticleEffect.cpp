#include "ParticleEffect.h"
#include "GameFramework.h"

ParticleEffect::ParticleEffect()
    : m_isActive(false), m_lifetime(0.0f), m_totalLifetime(1.0f) // 총 생존시간 0.5초
{
}

void ParticleEffect::Initialize(const ComPtr<ID3D12Device>& device, shared_ptr<MeshBase> mesh, shared_ptr<Shader> shader)
{
    m_object = make_shared<GameObject>(device);
    m_object->SetMesh(mesh);
    m_object->SetShader(shader);
    m_object->SetBaseColor(XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f)); // 노란색 기본
    m_object->SetScale(XMFLOAT3(0.1f, 0.1f, 0.1f)); // 기본 크기
}

void ParticleEffect::Activate(const XMFLOAT3& position)
{
    m_isActive = true;
    m_lifetime = 0.0f;
    m_object->SetPosition(position);
    m_object->SetVisible(true);

    float angle = XM_2PI * (rand() / (float)RAND_MAX);

    float horizontalSpeed = 20.0f + static_cast<float>(rand() % 200) / 2.0f; // 20~40
    float upwardSpeed = 5.0f + static_cast<float>(rand() % 100) / 2.0f;      // 5~15

    m_velocity = XMFLOAT3{
        cosf(angle) * horizontalSpeed,
        upwardSpeed,
        sinf(angle) * horizontalSpeed
    };

    m_startColor = XMFLOAT4(1.0f, 0.8f, 0.4f, 1.0f); // 밝은 불꽃
    // 끝 색은 진한 빨간색
    m_endColor = XMFLOAT4(1.0f, 0.2f, 0.0f, 0.0f); // 진한 불꽃 + 알파 0
    m_object->SetBaseColor(m_startColor);
}

void ParticleEffect::Deactivate()
{
    m_isActive = false;
    m_object->SetVisible(false);
}

void ParticleEffect::Update(float deltaTime)
{
    if (!m_isActive) return;

    m_lifetime += deltaTime;

    //위치 이동
    XMFLOAT3 pos = m_object->GetPosition();
    pos.x += m_velocity.x * deltaTime;
    pos.y += m_velocity.y * deltaTime;
    pos.z += m_velocity.z * deltaTime;
    m_object->SetPosition(pos);

    // 생존 비율
    float lifeRatio = m_lifetime / m_totalLifetime;
    lifeRatio = min(lifeRatio, 1.0f);

    float colorChangeRatio = min(lifeRatio / 0.3f, 1.0f);

    //알파 감소 (fade out)
    XMFLOAT4 currentColor;
    currentColor.x = m_startColor.x + (m_endColor.x - m_startColor.x) * colorChangeRatio;
    currentColor.y = m_startColor.y + (m_endColor.y - m_startColor.y) * colorChangeRatio;
    currentColor.z = m_startColor.z + (m_endColor.z - m_startColor.z) * colorChangeRatio;
    currentColor.w = m_startColor.w + (m_endColor.w - m_startColor.w) * lifeRatio;
    m_object->SetBaseColor(currentColor);

    if (m_lifetime >= m_totalLifetime)
    {
        Deactivate();
    }
}

void ParticleEffect::Render(const ComPtr<ID3D12GraphicsCommandList>& cmdList) const
{
    if (!m_isActive) return;
    m_object->Render(cmdList);
}

XMFLOAT3 ParticleEffect::GetPosition() const
{
    return m_object->GetPosition(); // 내부 GameObject의 위치를 반환
}
