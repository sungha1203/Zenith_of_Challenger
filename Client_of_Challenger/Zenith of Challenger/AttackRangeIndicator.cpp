#include "AttackRangeIndicator.h"

AttackRangeIndicator::AttackRangeIndicator(const ComPtr<ID3D12Device>& device)
    : GameObject(device)
{
    SetBaseColor({ 1.0f, 0.0f, 0.0f, 0.6f }); // 반투명 빨간색
    SetUseTexture(false); // 텍스처 없이 색상만
}

void AttackRangeIndicator::SetLifetime(float time)
{
    m_lifetime = time;
}

void AttackRangeIndicator::Update(float timeElapsed)
{
    m_elapsed += timeElapsed;

    // [1] FillAmount: 0부터 1까지 증가
    float fillSpeed = 1.0f / m_lifetime; // 예: 1.0 / 5.0 = 0.2
    m_fillAmount += timeElapsed * fillSpeed;
    m_fillAmount = std::min(1.0f, m_fillAmount);

    // [2] 일정 시간 후 사라지게 (Lifetime 기준)
    if (m_elapsed >= m_lifetime)
        SetVisible(false);

    float alpha = 1.0f - (m_elapsed / m_lifetime);
    alpha = std::clamp(alpha, 0.0f, 1.0f);
    SetBaseColor({ 1.0f, 0.0f, 0.0f, alpha }); // 페이드 아웃
}

bool AttackRangeIndicator::IsExpired() const
{
    return m_elapsed >= m_lifetime;
}

void AttackRangeIndicator::SetFillAmount(float value)
{
    m_fillAmount = value;
}
