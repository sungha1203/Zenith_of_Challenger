#pragma once
#include "Object.h"

class HealingObject : public GameObject
{
public:
    HealingObject(const ComPtr<ID3D12Device>& device);
    ~HealingObject() override = default;

    virtual void Update(FLOAT timeElapsed) override;

    void SetFloatAmplitude(float amplitude) { m_floatAmplitude = amplitude; }
    void SetFloatSpeed(float speed) { m_floatSpeed = speed; }
    void SetRotateSpeed(float degPerSec) { m_rotateSpeed = degPerSec; }
    void SetBasePosition(const XMFLOAT3& pos) { m_basePosition = pos; }

private:
    float m_totalTime = 0.0f;

    float m_floatAmplitude = 0.3f;   // 위아래 진폭
    float m_floatSpeed = 2.0f;       // 위아래 속도
    float m_rotateSpeed = 45.0f;     // 초당 Y축 회전 각도 (도)

    XMFLOAT3 m_basePosition{ 0, 0, 0 }; // 기준 위치
};
