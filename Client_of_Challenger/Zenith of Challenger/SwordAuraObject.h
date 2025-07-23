#pragma once
#include "Object.h"

class SwordAuraObject : public GameObject
{
public:
    SwordAuraObject(const ComPtr<ID3D12Device>& device);
    ~SwordAuraObject() override = default;

    void Update(FLOAT timeElapsed) override;
    virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) override;


private:

    float m_elapsedTime = 0.f;
};
