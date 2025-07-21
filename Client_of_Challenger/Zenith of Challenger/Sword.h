#pragma once
#pragma once
#include "Object.h"

class Sword : public GameObject
{
public:
    Sword(const ComPtr<ID3D12Device>& device);
    ~Sword() override = default;  
};
