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
    shared_ptr<GameObject> m_object; // ��ƼŬ �ϳ��� GameObject ���
    float m_lifetime; // ���� ����ִ� �ð�
    bool m_isActive;
    float m_totalLifetime; // �� ���� (ex: 1��)

    XMFLOAT3 m_velocity;   // ��ƼŬ Ȯ�� �ӵ�
    XMFLOAT4 m_color;      // ���� ���� ����
    XMFLOAT3 m_startScale;   // ���� ������
    XMFLOAT3 m_endScale;     // ���� �� ������
    XMFLOAT4 m_startColor; // ������ �� ���� ��
    XMFLOAT4 m_endColor;   // ���� �� ���� ��

};
