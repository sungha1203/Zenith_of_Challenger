#include "HealingEffectObject.h"

HealingEffectObject::HealingEffectObject(const ComPtr<ID3D12Device>& device)
    : GameObject(device)
{
	m_scale = { 1.0f, 1.0f, 1.0f };
}

void HealingEffectObject::Update(float timeElapsed, const std::shared_ptr<Camera>& camera)
{
    m_elapsed += timeElapsed;

    // [1] 타겟 위치 추적
    if (m_followTarget)
        SetPosition(m_followTarget->GetPosition());

    // [2] Billboard 처리
    if (camera)
    {
        XMFLOAT3 pos = GetPosition();
        XMFLOAT3 cameraRight = camera->GetU();  // 오른쪽 방향
        XMFLOAT3 cameraUp = camera->GetV();     // 위쪽 방향

        XMMATRIX world =
            XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z) *
            XMMATRIX(
                XMLoadFloat3(&cameraRight),
                XMLoadFloat3(&cameraUp),
                XMVectorSet(0.f, 0.f, 1.f, 0.f),
                XMVectorSet(pos.x, pos.y, pos.z, 1.f)
            );

        XMStoreFloat4x4(&m_worldMatrix, world);
    }

    // [3] Pulse + Fade
    float ratio = m_elapsed / m_lifetime;
    float pulse = 0.9f + 0.1f * sin(m_elapsed * 10.f);
    float alpha = 1.0f - ratio;

    m_scale = XMFLOAT3(0.2f * pulse, 0.2f * pulse, 0.2f);
    m_baseColor = XMFLOAT4(0.3f, 1.f, 0.3f, alpha);
}