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

private:
    float m_aliveTime = 0.0f;       // ���� ���� �ð�
    float m_lifetime = 3.0f;        // �ִ� ���� �ð�
    float m_speed = 15.0f;          // �̵� �ӵ�

    XMFLOAT3 m_direction = { 0, 0, 1 }; // �̵� ����
};
