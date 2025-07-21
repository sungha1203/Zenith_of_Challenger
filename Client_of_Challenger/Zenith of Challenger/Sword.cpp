#include"Sword.h"

Sword::Sword(const ComPtr<ID3D12Device>& device): GameObject(device)
{
	SetUseTexture(true);
}
