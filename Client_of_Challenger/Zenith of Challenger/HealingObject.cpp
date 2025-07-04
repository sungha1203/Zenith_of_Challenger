#include "HealingObject.h"
using namespace DirectX;

HealingObject::HealingObject(const ComPtr<ID3D12Device>& device)
    : GameObject(device)
{
    SetUseTexture(false); // 텍스처 없이 색상만
    SetBaseColor(XMFLOAT4(0.5f, 1.0f, 0.5f, 1.0f));
}

void HealingObject::Update(FLOAT timeElapsed)
{
    m_totalTime += timeElapsed;

    // [1] 기준 위치는 처음 위치 지정 후 SetPosition() 된 값을 사용
    if (m_totalTime < 0.05f) // 한 번만 기억
        m_basePosition = GetPosition();

    // [2] 회전 (Y축)
    float yaw = XMConvertToRadians(m_totalTime * m_rotateSpeed);
    XMMATRIX rot = XMMatrixRotationY(yaw);

    // [3] 부유 (Y축으로 sin 곡선)
    float yOffset = sinf(m_totalTime * m_floatSpeed) * m_floatAmplitude;
    XMFLOAT3 floatPos = m_basePosition;
    floatPos.y += yOffset;
    XMMATRIX trans = XMMatrixTranslation(floatPos.x, floatPos.y, floatPos.z);

    // [4] 스케일 적용
    XMFLOAT3 scaleVec = GetScale();
    XMMATRIX scale = XMMatrixScaling(scaleVec.x, scaleVec.y, scaleVec.z);

    // [5] 최종 월드 행렬: S * R * T
    XMMATRIX world = scale * rot * trans;
    SetWorldMatrix(world);
}
