#pragma once
#include "Object.h"

class MagicBall : public GameObject
{
public:
    MagicBall(const ComPtr<ID3D12Device>& device);
    ~MagicBall() override = default;

    virtual void Update(FLOAT timeElapsed) override;

    void SetDirection(const XMFLOAT3& dir) { m_direction = dir; }
    void SetSpeed(float speed) { m_speed = speed; }
    void SetLifetime(float time) { m_lifetime = time; }

    bool IsDead() const { return m_aliveTime >= m_lifetime; }

    void SetWaveOffsets(float offsetX, float offsetY)
    {
        m_waveOffsetX = offsetX;
        m_waveOffsetY = offsetY;
    }

    void SetScaleAnimation(float freqX, float ampX, float freqY, float ampY, float freqZ, float ampZ)
    {
        m_scaleFreqX = freqX;
        m_scaleAmpX = ampX;
        m_scaleFreqY = freqY;
        m_scaleAmpY = ampY;
        m_scaleFreqZ = freqZ;
        m_scaleAmpZ = ampZ;
    }

private:
    float m_aliveTime = 0.0f;       // 누적 생존 시간
    float m_lifetime = 3.0f;        // 최대 생존 시간
    float m_speed = 15.0f;          // 이동 속도

    float m_waveOffsetX = 0.0f;  // 좌우 흔들림 위상 차이
    float m_waveOffsetY = 0.0f;  // 상하 흔들림 위상 차이

    float m_scaleFreqX = 6.0f;
    float m_scaleFreqY = 8.0f;
    float m_scaleFreqZ = 5.0f;

    float m_scaleAmpX = 0.25f;
    float m_scaleAmpY = 0.15f;
    float m_scaleAmpZ = 0.2f;

    XMFLOAT3 m_direction = { 0, 0, 1 }; // 이동 방향
};
