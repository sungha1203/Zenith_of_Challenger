#pragma once
#pragma once
#pragma once
#include "Object.h"

class Shield : public GameObject
{
public:
    Shield(const ComPtr<ID3D12Device>& device);
    ~Shield() override = default;
};
