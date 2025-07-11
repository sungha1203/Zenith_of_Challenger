#pragma once
#include "Object.h"

class MagicBallTrail : public GameObject
{
public:
    MagicBallTrail(const ComPtr<ID3D12Device>& device);
    void Update(FLOAT timeElapsed) override;

    void SetLifetime(float life) { m_lifetime = life; }

private:
    float m_elapsed = 0.0f;
    float m_lifetime = 0.3f;
};
