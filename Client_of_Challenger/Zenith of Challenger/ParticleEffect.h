#pragma once
#include "stdafx.h"
#include "Object.h"

class ParticleEffect
{
public:
    ParticleEffect();
    void Initialize(const ComPtr<ID3D12Device>& device, shared_ptr<MeshBase> mesh, shared_ptr<Shader> shader);
    void Activate(const XMFLOAT3& position);
    void Deactivate();
    void Update(float deltaTime);
    void Render(const ComPtr<ID3D12GraphicsCommandList>& cmdList) const;
    bool IsAlive() const { return m_isActive; }
    XMFLOAT3 GetPosition() const;
private:
    shared_ptr<GameObject> m_object; // 파티클 하나당 GameObject 사용
    float m_lifetime; // 현재 살아있는 시간
    bool m_isActive;
    float m_totalLifetime; // 총 수명 (ex: 1초)

    XMFLOAT3 m_velocity;   // 파티클 확산 속도
    XMFLOAT4 m_color;      // 색상 알파 포함
    XMFLOAT3 m_startScale;   // 시작 스케일
    XMFLOAT3 m_endScale;     // 끝날 때 스케일
    XMFLOAT4 m_startColor; // 시작할 때 밝은 색
    XMFLOAT4 m_endColor;   // 끝날 때 진한 색

};
