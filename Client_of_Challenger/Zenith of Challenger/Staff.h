#pragma once
#pragma once
#include "Object.h"

class Staff : public GameObject
{
public:
    Staff(const ComPtr<ID3D12Device>& device);
    ~Staff() override = default;
};
