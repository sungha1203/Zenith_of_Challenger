#include "SwordAuraObject.h"

SwordAuraObject::SwordAuraObject(const ComPtr<ID3D12Device>& device)
    : GameObject(device)
{
    SetBaseColor(XMFLOAT4(0.2f, 0.6f, 1.f, 0.4f)); // �������� Ǫ�� ����
    SetUseTexture(false);
    SetScale(XMFLOAT3(2.1f, 2.1f, 2.1f)); // ���⺸�� �ణ ũ��
}

void SwordAuraObject::Update(FLOAT timeElapsed)
{
    m_elapsedTime += timeElapsed;
    PlusRotationY(timeElapsed * 90.f); // õõ�� ȸ��


}

void SwordAuraObject::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    GameObject::Render(commandList);
}
