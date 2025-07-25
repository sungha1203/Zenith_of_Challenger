#include"Shield.h"

Shield::Shield(const ComPtr<ID3D12Device>& device) : GameObject(device)
{
	SetUseTexture(true);
}
