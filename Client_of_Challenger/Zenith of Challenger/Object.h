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
	void SetRotationX(float Roll);

	void PlusRotationY(float yaw);
	void PlusRotationZ(float pitch);
	void RotationX90();
	void RotationY90();
	void RotationZ90();
	virtual void SetScale(XMFLOAT3 scale);
	virtual XMFLOAT3 GetScale() const;
	XMFLOAT3			m_prevPosition;
	XMFLOAT3            m_scale;
protected:
	XMFLOAT4X4			m_worldMatrix;

	XMFLOAT3			m_right;
	XMFLOAT3			m_up;
	XMFLOAT3			m_front;

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
	XMFLOAT4X4 worldMatrix;    // c0~c3
	XMFLOAT4   baseColor;      // c4

	UINT       useTexture;     // c5.x
	UINT       textureIndex;   // c5.y
	UINT       isHovered;      // c5.z
	FLOAT      fillAmount;     // c5.w

	XMFLOAT4   customUV;       // c6
	UINT       useCustomUV;    // c7.x
	FLOAT      totalTime;      // c7.y
	XMFLOAT2   padding;        // c7.z~w 정렬용

	XMFLOAT3   dissolveAxis;   // c8.x~z
	FLOAT      dissolveAmount; // c8.w

	XMFLOAT3   dissolveOrigin; // c9.x~z
	FLOAT      dissolvePadding; // c9.w (정렬)

	XMFLOAT3   padding2;       // c10.x~z (총 16바이트 정렬용)
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

	//버튼 클릭 관련
	bool GameObject::IsPointInside(int mouseX, int mouseY) const;

	//툰렌더링 외곽선용
	virtual void RenderOutline(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;
	void SetOutlineShader(const std::shared_ptr<Shader>& shader) { m_outlineShader = shader; }

	//바운딩 박스 따로 스케일 정의
	void RenderBoundingBoxWithoutScale(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;


	void SetActive(bool active) { m_isActive = active; }
	bool IsActive() const { return m_isActive; }

	const XMFLOAT4X4& GetWorldMatrix() const { return m_worldMatrix; }

	void MarkDead() { m_isDead = true; }
	bool IsDead() const { return m_isDead; }

	BoundingBox GetWorldBoundingBox() const;

	int m_ownerJob = 0;

	// 디졸브 셰이더용 설정 함수
	void SetDissolveAxis(XMFLOAT3 axis) { m_dissolveAxis = axis; }
	void SetDissolveOrigin(XMFLOAT3 origin) { m_dissolveOrigin = origin; }
	void SetDissolveAmount(float amount) { m_dissolveAmount = amount; }

	XMFLOAT3 GetDissolveAxis() const { return m_dissolveAxis; }
	XMFLOAT3 GetDissolveOrigin() const { return m_dissolveOrigin; }
	float GetDissolveAmount() const { return m_dissolveAmount; }

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
	shared_ptr<Mesh<DebugVertex>> m_AttdebugBoxMesh;
	bool m_drawBoundingBox = false;
	shared_ptr<Shader> m_debugLineShader; //와이어프레임 전용 셰이더

	shared_ptr<Shader> m_HealthBarShader; //일반 몬스터 HP바 전용 셰이더

	//골드 스코어 전용
	XMFLOAT4 m_customUV{ 0.f, 0.f, 1.f, 1.f };
	UINT m_useCustomUV = 0;

	//외곽선 전용
	shared_ptr<Shader> m_outlineShader;

	bool m_isActive = true; // 기본값은 true
	bool m_isDead = false;


	//디졸브 셰이더를 위한 상수 버퍼 값
	XMFLOAT3 m_dissolveAxis{ 0.f, 1.f, 0.f };   // 기본은 Y축
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