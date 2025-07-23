#include "SwordAuraObject.h"

SwordAuraObject::SwordAuraObject(const ComPtr<ID3D12Device>& device)
    : GameObject(device)
{
    SetBaseColor(XMFLOAT4(0.2f, 0.6f, 1.f, 0.4f)); // 반투명한 푸른 오라
    SetUseTexture(false);
    SetScale(XMFLOAT3(2.1f, 2.1f, 2.1f)); // 무기보다 약간 크게
}

void SwordAuraObject::Update(FLOAT timeElapsed)
{
    m_elapsedTime += timeElapsed;
    PlusRotationY(timeElapsed * 90.f); // 천천히 회전


}

void SwordAuraObject::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    GameObject::Render(commandList);
}
