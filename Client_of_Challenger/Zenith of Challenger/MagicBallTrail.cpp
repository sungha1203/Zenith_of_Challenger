#include "MagicBallTrail.h"

MagicBallTrail::MagicBallTrail(const ComPtr<ID3D12Device>& device)
    : GameObject(device)
{
    SetUseTexture(false); // 발광 텍스처 안 써도 됨
    SetScale(XMFLOAT3(0.2f, 0.2f, 0.2f)); // 기본 크기
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

    // 투명도
    float alpha = 1.0f - t * 0.9f;

    // 스케일 점점 작게
    float factor = 1.0f - t;
    float shrink = 0.6f + 0.4f * factor; // 처음 1.0 마지막 0.6배

    SetBaseColor(XMFLOAT4(1.0f, 0.2f, 1.0f, alpha));
    SetScale(XMFLOAT3(
        m_scale.x * shrink,
        m_scale.y * shrink,
        m_scale.z * shrink
    ));
}
