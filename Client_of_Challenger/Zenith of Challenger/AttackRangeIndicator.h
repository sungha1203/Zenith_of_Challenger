#pragma once
#include "Object.h"

class AttackRangeIndicator : public GameObject
{
public:
    AttackRangeIndicator(const ComPtr<ID3D12Device>& device);

    void Update(float timeElapsed) override;
    void SetLifetime(float time);
    bool IsExpired() const;
    void SetFillAmount(float value);

private:
    float m_elapsed = 0.f;
    float m_lifetime = 5.0f; // ±‚∫ª 1.5√ 
};
