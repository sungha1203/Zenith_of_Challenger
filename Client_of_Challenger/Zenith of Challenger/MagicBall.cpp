#include "MagicBall.h"
using namespace DirectX;

MagicBall::MagicBall(const ComPtr<ID3D12Device>& device)
    : GameObject(device)
{
    SetUseTexture(false); // �ؽ�ó ���� ����� ǥ��
    SetBaseColor(XMFLOAT4(0.7f, 0.3f, 1.0f, 1.0f));
    SetScale(XMFLOAT3(0.2f, 0.2f, 0.2f));
}

void MagicBall::Update(FLOAT timeElapsed)
{
    m_aliveTime += timeElapsed;
    if (IsDead()) return;

    XMFLOAT3 pos = GetPosition();
    pos.x += m_direction.x * m_speed * timeElapsed;
    pos.y += m_direction.y * m_speed * timeElapsed;
    pos.z += m_direction.z * m_speed * timeElapsed;

    float waveX = sinf(m_aliveTime * 10.0f + m_waveOffsetX);
    float waveY = cosf(m_aliveTime * 7.0f + m_waveOffsetY);
    float waveAmp = 0.25f;                         // ���� (���� ����)

    pos.x += waveX * waveAmp;  // �¿� ��鸲
    pos.y += waveY * waveAmp;  // ���� ��鸲

    SetPosition(pos);

    float yaw = XMConvertToRadians(m_aliveTime * 360.0f); // �ʴ� 1ȸ��
    XMMATRIX rot = XMMatrixRotationY(yaw);

    float pulseX = 1.0f + m_scaleAmpX * sinf(m_aliveTime * m_scaleFreqX);
    float pulseY = 1.0f + m_scaleAmpY * cosf(m_aliveTime * m_scaleFreqY);
    float pulseZ = 1.0f + m_scaleAmpZ * sinf(m_aliveTime * m_scaleFreqZ);

    XMMATRIX scl = XMMatrixScaling(
        pulseX * m_scale.x,
        pulseY * m_scale.y,
        pulseZ * m_scale.z
    );

    XMMATRIX trn = XMMatrixTranslation(pos.x, pos.y, pos.z);
    SetWorldMatrix(scl * rot * trn);
}

