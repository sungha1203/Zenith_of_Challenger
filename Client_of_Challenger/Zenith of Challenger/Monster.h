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

class MushroomDark : public Monsters
{
public:
    MushroomDark(const ComPtr<ID3D12Device>& device) : Monsters(device)
    {
        SetScale(XMFLOAT3(1.0f, 1.0f, 1.0f));
    }
};

class VenusBlue : public Monsters
{
public:
    VenusBlue(const ComPtr<ID3D12Device>& device) : Monsters(device)
    {
        SetScale(XMFLOAT3(1.0f, 1.0f, 1.0f));
    }
};

class PlantDionaea : public Monsters
{
public:
    PlantDionaea(const ComPtr<ID3D12Device>& device) : Monsters(device)
    {
        SetScale(XMFLOAT3(1.0f, 1.0f, 1.0f));
    }
};

class Boss : public Monsters
{
public:
    Boss(const ComPtr<ID3D12Device>& device) : Monsters(device)
    {
        SetScale(XMFLOAT3(1.0f, 1.0f, 1.0f));
    }
};