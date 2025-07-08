#include "MagicBall.h"
using namespace DirectX;

MagicBall::MagicBall(const ComPtr<ID3D12Device>& device)
    : GameObject(device)
{
    SetUseTexture(false); // 텍스처 없이 보라색 표현
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

    float waveX = sinf(m_aliveTime * 10.0f);       // 좌우
    float waveY = cosf(m_aliveTime * 7.0f);        // 상하
    float waveAmp = 0.25f;                         // 진폭 (진동 강도)

    pos.x += waveX * waveAmp;  // 좌우 흔들림
    pos.y += waveY * waveAmp;  // 상하 흔들림

    SetPosition(pos);

    float yaw = XMConvertToRadians(m_aliveTime * 360.0f); // 초당 1회전
    XMMATRIX rot = XMMatrixRotationY(yaw);

    float pulseX = 1.0f + 0.25f * sinf(m_aliveTime * 6.0f);   // X축
    float pulseY = 1.0f + 0.15f * cosf(m_aliveTime * 8.0f);   // Y축
    float pulseZ = 1.0f + 0.2f * sinf(m_aliveTime * 5.0f);    // Z축

    XMMATRIX scl = XMMatrixScaling(
        pulseX * m_scale.x,
        pulseY * m_scale.y,
        pulseZ * m_scale.z
    );

    XMMATRIX trn = XMMatrixTranslation(pos.x, pos.y, pos.z);
    SetWorldMatrix(scl * rot * trn);
}

