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

    float m_floatAmplitude = 0.3f;   // ���Ʒ� ����
    float m_floatSpeed = 2.0f;       // ���Ʒ� �ӵ�
    float m_rotateSpeed = 45.0f;     // �ʴ� Y�� ȸ�� ���� (��)

    XMFLOAT3 m_basePosition{ 0, 0, 0 }; // ���� ��ġ
};
