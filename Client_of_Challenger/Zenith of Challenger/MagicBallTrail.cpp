#include "MagicBallTrail.h"

MagicBallTrail::MagicBallTrail(const ComPtr<ID3D12Device>& device)
    : GameObject(device)
{
    SetUseTexture(false); // �߱� �ؽ�ó �� �ᵵ ��
    SetScale(XMFLOAT3(0.2f, 0.2f, 0.2f)); // �⺻ ũ��
}

void MagicBallTrail::Update(FLOAT timeElapsed)
{
    m_elapsed += timeElapsed;
    if (m_elapsed >= m_lifetime)
    {
        MarkDead();
        return;
    }

    float t = m_elapsed / m_lifetime;

    // ����
    float alpha = 1.0f - t * 0.9f;

    // ������ ���� �۰�
    float factor = 1.0f - t;
    float shrink = 0.6f + 0.4f * factor; // ó�� 1.0 ������ 0.6��

    SetBaseColor(XMFLOAT4(1.0f, 0.2f, 1.0f, alpha));
    SetScale(XMFLOAT3(
        m_scale.x * shrink,
        m_scale.y * shrink,
        m_scale.z * shrink
    ));
}
