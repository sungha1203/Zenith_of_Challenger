#pragma once
#include "Monsters.h"

class Frightfly : public Monsters
{
public:
    Frightfly(const ComPtr<ID3D12Device>& device) : Monsters(device){}
};

class FlowerFairy : public Monsters
{
public:
    FlowerFairy(const ComPtr<ID3D12Device>& device) : Monsters(device)
    {
        SetScale(XMFLOAT3(1.0f, 1.0f, 1.0f));
    }
};

class PlantDionae : public Monsters
{
public:
    PlantDionae(const ComPtr<ID3D12Device>& device) : Monsters(device)
    {
        SetScale(XMFLOAT3(0.9f, 0.9f, 0.9f));
    }
};

class PlantVenus : public Monsters
{
public:
    PlantVenus(const ComPtr<ID3D12Device>& device) : Monsters(device)
    {
        SetScale(XMFLOAT3(1.1f, 1.1f, 1.1f));
    }
};
