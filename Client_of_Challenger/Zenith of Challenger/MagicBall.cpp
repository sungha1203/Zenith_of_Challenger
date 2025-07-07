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
    if (IsDead())
    {
        return;
    }

    // 위치 이동
    XMFLOAT3 currPos = GetPosition();
    currPos.x += m_direction.x * m_speed * timeElapsed;
    currPos.y += m_direction.y * m_speed * timeElapsed;
    currPos.z += m_direction.z * m_speed * timeElapsed;
    SetPosition(currPos);

    // 회전 (옵션)
    float yaw = XMConvertToRadians(m_aliveTime * 360.0f);
    XMMATRIX rot = XMMatrixRotationY(yaw);

    // 스케일/회전/이동 적용
    XMMATRIX scl = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
    XMMATRIX trn = XMMatrixTranslation(currPos.x, currPos.y, currPos.z);

    SetWorldMatrix(scl * rot * trn);
}
