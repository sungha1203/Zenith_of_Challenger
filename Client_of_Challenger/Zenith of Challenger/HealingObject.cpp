#include "HealingObject.h"
using namespace DirectX;

HealingObject::HealingObject(const ComPtr<ID3D12Device>& device)
    : GameObject(device)
{
    SetUseTexture(false); // �ؽ�ó ���� ����
    SetBaseColor(XMFLOAT4(0.5f, 1.0f, 0.5f, 1.0f));
}

void HealingObject::Update(FLOAT timeElapsed)
{
    m_totalTime += timeElapsed;

    // [1] ���� ��ġ�� ó�� ��ġ ���� �� SetPosition() �� ���� ���
    if (m_totalTime < 0.05f) // �� ���� ���
        m_basePosition = GetPosition();

    // [2] ȸ�� (Y��)
    float yaw = XMConvertToRadians(m_totalTime * m_rotateSpeed);
    XMMATRIX rot = XMMatrixRotationY(yaw);

    // [3] ���� (Y������ sin �)
    float yOffset = sinf(m_totalTime * m_floatSpeed) * m_floatAmplitude;
    XMFLOAT3 floatPos = m_basePosition;
    floatPos.y += yOffset;
    XMMATRIX trans = XMMatrixTranslation(floatPos.x, floatPos.y, floatPos.z);

    // [4] ������ ����
    XMFLOAT3 scaleVec = GetScale();
    XMMATRIX scale = XMMatrixScaling(scaleVec.x, scaleVec.y, scaleVec.z);

    // [5] ���� ���� ���: S * R * T
    XMMATRIX world = scale * rot * trans;
    SetWorldMatrix(world);
}
