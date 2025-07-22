//------------------------------------------------------- ----------------------
// File: Object.h
//-----------------------------------------------------------------------------

#pragma once
#include "stdafx.h"
#include "mesh.h"
#include "texture.h"
#include "buffer.h"
#include "Material.h"
#include "Lighting.h"
#include "Shader.h"

class Object abstract
{
public:
	Object();

	virtual void Update(FLOAT timeElapsed) = 0;

	void Transform(XMFLOAT3 shift);
	void Rotate(FLOAT pitch, FLOAT yaw, FLOAT roll);

	void SetPosition(XMFLOAT3 position);
	XMFLOAT3 GetPosition() const;
	XMFLOAT3 GetRotation() const { return m_rotation; }

	void UpdateWorldMatrix();
	void SetRotationY(float yaw);
	void SetRotationZ(float pitch);
	void SetRotationX(float Roll);
	float GetRotationX() const { return m_rotation.x; }
	float GetRotationY() const { return m_rotation.y; }
	float GetRotationZ() const { return m_rotation.z; }

	void PlusRotationY(float yaw);
	void PlusRotationZ(float pitch);
	void RotationX90();
	void RotationY90();
	void RotationZ90();
	virtual void SetScale(XMFLOAT3 scale);
	virtual XMFLOAT3 GetScale() const;
	XMFLOAT3			m_prevPosition;
	XMFLOAT3            m_scale;
	//XMFLOAT4X4 GetWorldMatrix() { return m_worldMatrix; }
	//void SetWorldMatrix(XMFLOAT4X4 mat) { m_worldMatrix = mat;}
protected:
	XMFLOAT4X4			m_worldMatrix;

	XMFLOAT3			m_right;
	XMFLOAT3			m_up;
	XMFLOAT3			m_front;

	XMFLOAT3			m_position; // ��ġ�� ���� ����
	XMFLOAT3			m_rotation{ 0.0f,0.0f, 0.0f }; // ���⿡ ȸ�� ���� �߰� (pitch, yaw, roll)
};


struct InstanceData;
class InstanceObject : public Object
{
public:
	InstanceObject();
	virtual ~InstanceObject() = default;

	virtual void Update(FLOAT timeElapsed) override;
	void UpdateShaderVariable(InstanceData& buffer);

	void SetTextureIndex(UINT textureIndex);
	void SetMaterialIndex(UINT materialIndex);

	//FBX �޽� ���� �Լ� �߰�
	void SetMesh(const shared_ptr<MeshBase>& mesh) { m_mesh = mesh; }
protected:
	UINT				m_textureIndex;
	UINT				m_materialIndex;
	shared_ptr<MeshBase> m_mesh; //FBX ���� ������ ���� �߰�
};

struct ObjectData : public BufferBase
{
	XMFLOAT4X4 worldMatrix;    // c0~c3
	XMFLOAT4   baseColor;      // c4

	UINT       useTexture;     // c5.x
	UINT       textureIndex;   // c5.y
	UINT       isHovered;      // c5.z
	FLOAT      fillAmount;     // c5.w

	XMFLOAT4   customUV;       // c6
	UINT       useCustomUV;    // c7.x
	FLOAT      totalTime;      // c7.y
	XMFLOAT2   padding;        // c7.z~w ���Ŀ�

	XMFLOAT3   dissolveAxis;   // c8.x~z
	FLOAT      dissolveAmount; // c8.w

	XMFLOAT3   dissolveOrigin; // c9.x~z
	FLOAT      dissolvePadding; // c9.w (����)

	XMFLOAT3   padding2;       // c10.x~z (�� 16����Ʈ ���Ŀ�)
};

class GameObject : public Object
{
public:
	GameObject(const ComPtr<ID3D12Device>& device);
	virtual ~GameObject() = default;

	virtual void Update(FLOAT timeElapsed);
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList);
	virtual void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList, const XMFLOAT4X4* overrideMatrix = nullptr) const;

	void SetMesh(const shared_ptr<MeshBase>& mesh);
	shared_ptr<MeshBase> GetMesh() const { return m_mesh; }

	void SetTexture(const shared_ptr<Texture>& texture);
	void SetMaterial(const shared_ptr<Material>& material);
	void SetShader(const shared_ptr<Shader>& shader);
	void SetBaseColor(const XMFLOAT4& color);            // �߰�
	void SetUseTexture(bool use);                        // �߰�
	const XMFLOAT4& GetBaseColor() const { return m_baseColor; }


	void SetTextureIndex(int index) { m_textureIndex = index; }

	virtual void SetHP(float currentHP, float maxHP);

	int GetTextureIndex() const { return m_textureIndex; }

	// ��ȯ ��� ���� �Լ� �߰�
	void SetWorldMatrix(const XMMATRIX& worldMatrix);

	void SetSRV(D3D12_GPU_DESCRIPTOR_HANDLE srvHandle);

	// �� ���ý� �ʿ��� �ɹ�
	void SetHovered(bool hovered) { m_isHovered = hovered; }
	bool IsHovered() const { return m_isHovered; }

	void SetVisible(bool visible) { m_isVisible = visible; }
	bool IsVisible() const { return m_isVisible; }

	//�浹üũ ��
	BoundingBox GetBoundingBox() const { return m_boundingBox; }
	void SetBoundingBox(const BoundingBox& box);
	void SetPlayerBoundingBox(const BoundingBox& box);
	void SetMonstersBoundingBox(const BoundingBox& box);
	void SetFlyFrightBoundingBox(const BoundingBox& box);
	void SetDebugLineShader(const shared_ptr<Shader>& shader) { m_debugLineShader = shader; }
	void SetDrawBoundingBox(bool draw) { m_drawBoundingBox = draw; }
	bool IsDrawBoundingBox() const { return m_drawBoundingBox; }
	int m_textureIndex = 0;
	float m_fillAmount = 1.0f;


	void SetHealthBarShader(const shared_ptr<Shader>& shader) { m_HealthBarShader = shader; }

	void SetCustomUV(float u0, float v0, float u1, float v1);
	void SetuseCustomUV(int useUV);

	//��ư Ŭ�� ����
	bool GameObject::IsPointInside(int mouseX, int mouseY) const;

	//�������� �ܰ�����
	virtual void RenderOutline(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;
	void SetOutlineShader(const std::shared_ptr<Shader>& shader) { m_outlineShader = shader; }

	//�ٿ�� �ڽ� ���� ������ ����
	void RenderBoundingBoxWithoutScale(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;


	void SetActive(bool active) { m_isActive = active; }
	bool IsActive() const { return m_isActive; }

	const XMFLOAT4X4& GetWorldMatrix() const { return m_worldMatrix; }

	void MarkDead() { m_isDead = true; }
	bool IsDead() const { return m_isDead; }

	BoundingBox GetWorldBoundingBox() const;

	int m_ownerJob = 0;

	// ������ ���̴��� ���� �Լ�
	void SetDissolveAxis(XMFLOAT3 axis) { m_dissolveAxis = axis; }
	void SetDissolveOrigin(XMFLOAT3 origin) { m_dissolveOrigin = origin; }
	void SetDissolveAmount(float amount) { m_dissolveAmount = amount; }

	XMFLOAT3 GetDissolveAxis() const { return m_dissolveAxis; }
	XMFLOAT3 GetDissolveOrigin() const { return m_dissolveOrigin; }
	float GetDissolveAmount() const { return m_dissolveAmount; }

	void SetTempWorldMatrix(const XMMATRIX& mat) { m_tempWorldMatrix = mat; } 
	const XMMATRIX& GetTempWorldMatrix() const { return m_tempWorldMatrix; }
protected:
	XMMATRIX m_tempWorldMatrix = XMMatrixIdentity(); 
	shared_ptr<MeshBase> m_mesh;
	shared_ptr<Texture> m_texture;
	shared_ptr<Material> m_material;
	shared_ptr<Shader> m_shader;

	unique_ptr<UploadBuffer<ObjectData>> m_constantBuffer;

	XMFLOAT4 m_baseColor{ 1.f, 1.f, 1.f, 1.f };           // �⺻ ����
	BOOL m_useTexture = FALSE;                           // �ؽ�ó ��� ����

	D3D12_GPU_DESCRIPTOR_HANDLE m_srvHandle{};  // �ؽ�ó SRV �ڵ�
	bool m_isHovered = false; //��ư �̺�Ʈ �Һ�
	bool m_isVisible = true; //���� ��ư�� ���� �� ������ ��ŸƮ ��ư ��


	//�浹üũ ��
	BoundingBox m_boundingBox;
	shared_ptr<Mesh<DebugVertex>> m_debugBoxMesh;
	shared_ptr<Mesh<DebugVertex>> m_AttdebugBoxMesh;
	bool m_drawBoundingBox = false;
	shared_ptr<Shader> m_debugLineShader; //���̾������� ���� ���̴�

	shared_ptr<Shader> m_HealthBarShader; //�Ϲ� ���� HP�� ���� ���̴�

	//��� ���ھ� ����
	XMFLOAT4 m_customUV{ 0.f, 0.f, 1.f, 1.f };
	UINT m_useCustomUV = 0;

	//�ܰ��� ����
	shared_ptr<Shader> m_outlineShader;

	bool m_isActive = true; // �⺻���� true
	bool m_isDead = false;


	//������ ���̴��� ���� ��� ���� ��
	XMFLOAT3 m_dissolveAxis{ 0.f, 1.f, 0.f };   // �⺻�� Y��
	float    m_dissolveAmount = 0.f;
	XMFLOAT3 m_dissolveOrigin{ 0.f, 0.f, 0.f };

};

class RotatingObject : public InstanceObject
{
public:
	RotatingObject();
	~RotatingObject() override = default;

	void Update(FLOAT timeElapsed) override;

private:
	FLOAT m_rotatingSpeed;
};

class LightObject : public RotatingObject
{
public:
	LightObject(const shared_ptr<SpotLight>& light);
	~LightObject() override = default;

	virtual void Update(FLOAT timeElapsed) override;
private:
	shared_ptr<SpotLight> m_light;
};

class Sun : public InstanceObject
{
public:
	Sun(const shared_ptr<DirectionalLight>& light);
	~Sun() override = default;

	void SetStrength(XMFLOAT3 strength);

	void Update(FLOAT timeElapsed) override;

private:
	shared_ptr<DirectionalLight> m_light;

	XMFLOAT3 m_strength;
	FLOAT m_phi;
	FLOAT m_theta;
	const FLOAT m_radius;
};

class Terrain : public GameObject
{
public:
	Terrain(const ComPtr<ID3D12Device>& device);
	~Terrain() override = default;

	FLOAT GetHeight(FLOAT x, FLOAT z);
};