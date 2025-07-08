#include "MagicBall.h"
using namespace DirectX;

MagicBall::MagicBall(const ComPtr<ID3D12Device>& device)
    : GameObject(device)
{
    SetUseTexture(false); // �ؽ�ó ���� ����� ǥ��
    SetBaseColor(XMFLOAT4(0.7f, 0.3f, 1.0f, 1.0f));
    SetScale(XMFLOAT3(0.1f, 0.1f, 0.1f));
}

void MagicBall::Update(FLOAT timeElapsed)
{
    m_aliveTime += timeElapsed;
    if (IsDead()) return;

    XMFLOAT3 pos = GetPosition();
    pos.x += m_direction.x * m_speed * timeElapsed;
    pos.y += m_direction.y * m_speed * timeElapsed;
    pos.z += m_direction.z * m_speed * timeElapsed;

    float waveX = sinf(m_aliveTime * 10.0f);       // �¿�
    float waveY = cosf(m_aliveTime * 7.0f);        // ����
    float waveAmp = 0.25f;                         // ���� (���� ����)

    pos.x += waveX * waveAmp;  // �¿� ��鸲
    pos.y += waveY * waveAmp;  // ���� ��鸲

    SetPosition(pos);

    float yaw = XMConvertToRadians(m_aliveTime * 360.0f); // �ʴ� 1ȸ��
    XMMATRIX rot = XMMatrixRotationY(yaw);

    float pulseX = 1.0f + 0.25f * sinf(m_aliveTime * 6.0f);   // X��
    float pulseY = 1.0f + 0.15f * cosf(m_aliveTime * 8.0f);   // Y��
    float pulseZ = 1.0f + 0.2f * sinf(m_aliveTime * 5.0f);    // Z��

    XMMATRIX scl = XMMatrixScaling(
        pulseX * m_scale.x,
        pulseY * m_scale.y,
        pulseZ * m_scale.z
    );

    XMMATRIX trn = XMMatrixTranslation(pos.x, pos.y, pos.z);
    SetWorldMatrix(scl * rot * trn);
}

