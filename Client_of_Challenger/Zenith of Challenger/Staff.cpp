#include"Staff.h"

Staff::Staff(const ComPtr<ID3D12Device>& device) : GameObject(device)
{
	SetUseTexture(true);
}
