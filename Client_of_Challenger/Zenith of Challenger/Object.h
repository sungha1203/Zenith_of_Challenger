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

	void UpdateWorldMatrix();
	void SetRotationY(float yaw);
	void SetRotationZ(float pitch);

	void PlusRotationY(float yaw);
	void PlusRotationZ(float pitch);
	void RotationX90();
	void RotationY90();
	void RotationZ90();
	virtual void SetScale(XMFLOAT3 scale);
	virtual XMFLOAT3 GetScale() const;
	XMFLOAT3			m_prevPosition;
protected:
	XMFLOAT4X4			m_worldMatrix;

	XMFLOAT3			m_right;
	XMFLOAT3			m_up;
	XMFLOAT3			m_front;

	XMFLOAT3            m_scale;
	XMFLOAT3			m_position; // 위치값 따로 보관
	XMFLOAT3			m_rotation{ 0.0f,0.0f, 0.0f }; // 여기에 회전 정보 추가 (pitch, yaw, roll)
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

	//FBX 메쉬 설정 함수 추가
	void SetMesh(const shared_ptr<MeshBase>& mesh) { m_mesh = mesh; }
protected:
	UINT				m_textureIndex;
	UINT				m_materialIndex;
	shared_ptr<MeshBase> m_mesh; //FBX 모델을 저장할 변수 추가
};

struct ObjectData : public BufferBase
{
	XMFLOAT4X4 worldMatrix;    // 64 bytes
	XMFLOAT4 baseColor;        // 16 bytes
	UINT useTexture;           // 4 bytes
	UINT textureIndex;         // 4 bytes
	UINT isHovered;			   // 4 bytes
	FLOAT fillAmount = {};     // 12 bytes → 총 32 bytes로 16바이트 정렬 유지
	XMFLOAT4 customUV;
	UINT useCustomUV;
};

class GameObject : public Object
{
public:
	GameObject(const ComPtr<ID3D12Device>& device);
	virtual ~GameObject() = default;

	virtual void Update(FLOAT timeElapsed);
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;
	virtual void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList, const XMFLOAT4X4* overrideMatrix = nullptr) const;

	void SetMesh(const shared_ptr<MeshBase>& mesh);
	void SetTexture(const shared_ptr<Texture>& texture);
	void SetMaterial(const shared_ptr<Material>& material);
	void SetShader(const shared_ptr<Shader>& shader);
	void SetBaseColor(const XMFLOAT4& color);            // 추가
	void SetUseTexture(bool use);                        // 추가

	void SetTextureIndex(int index) { m_textureIndex = index; }

	virtual void SetHP(float currentHP, float maxHP);

	int GetTextureIndex() const { return m_textureIndex; }

	// 변환 행렬 설정 함수 추가
	void SetWorldMatrix(const XMMATRIX& worldMatrix);

	void SetSRV(D3D12_GPU_DESCRIPTOR_HANDLE srvHandle);

	// 방 선택시 필요한 맴버
	void SetHovered(bool hovered) { m_isHovered = hovered; }
	bool IsHovered() const { return m_isHovered; }

	void SetVisible(bool visible) { m_isVisible = visible; }
	bool IsVisible() const { return m_isVisible; }

	//충돌체크 용
	BoundingBox GetBoundingBox() const { return m_boundingBox; }
	void SetBoundingBox(const BoundingBox& box);
	void SetDebugLineShader(const shared_ptr<Shader>& shader) { m_debugLineShader = shader; }
	void SetDrawBoundingBox(bool draw) { m_drawBoundingBox = draw; }
	bool IsDrawBoundingBox() const { return m_drawBoundingBox; }
	int m_textureIndex = 0;
	float m_fillAmount = 2.0f;


	void SetHealthBarShader(const shared_ptr<Shader>& shader) { m_HealthBarShader = shader; }

	void SetCustomUV(float u0, float v0, float u1, float v1);
	void SetuseCustomUV(int useUV);

	//버튼 클릭 관련
	bool GameObject::IsPointInside(int mouseX, int mouseY) const;

	//툰렌더링 외곽선용
	virtual void RenderOutline(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;
	void SetOutlineShader(const std::shared_ptr<Shader>& shader) { m_outlineShader = shader; }

protected:
	shared_ptr<MeshBase> m_mesh;
	shared_ptr<Texture> m_texture;
	shared_ptr<Material> m_material;
	shared_ptr<Shader> m_shader;

	unique_ptr<UploadBuffer<ObjectData>> m_constantBuffer;

	XMFLOAT4 m_baseColor{ 1.f, 1.f, 1.f, 1.f };           // 기본 색상
	BOOL m_useTexture = FALSE;                           // 텍스처 사용 여부

	D3D12_GPU_DESCRIPTOR_HANDLE m_srvHandle{};  // 텍스처 SRV 핸들
	bool m_isHovered = false; //버튼 이벤트 불빛
	bool m_isVisible = true; //참가 버튼을 누른 후 나오는 스타트 버튼 용


	//충돌체크 용
	BoundingBox m_boundingBox;
	shared_ptr<Mesh<DebugVertex>> m_debugBoxMesh;
	bool m_drawBoundingBox = false;
	shared_ptr<Shader> m_debugLineShader; //와이어프레임 전용 셰이더

	shared_ptr<Shader> m_HealthBarShader; //일반 몬스터 HP바 전용 셰이더

	//골드 스코어 전용
	XMFLOAT4 m_customUV{ 0.f, 0.f, 1.f, 1.f };
	UINT m_useCustomUV = 0;

	//외곽선 전용
	shared_ptr<Shader> m_outlineShader;
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