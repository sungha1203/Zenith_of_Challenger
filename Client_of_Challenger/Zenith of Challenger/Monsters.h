#pragma once
#include "object.h"
#include "camera.h"
#include "Animation.h"
#include "d3dUtil.h"
#include "Shader.h"            // Shader�� shared_ptr�� �����ϰ� ����

class HealthBarObject;

enum class NormalMonsterType
{
	Mushroom,
	FightFly,
	PlantDionaea,
	PeaShooter,
	PlantVenus,
	FlowerFairy
};

class Monsters : public GameObject
{
public:
	Monsters(const ComPtr<ID3D12Device>& device);
	~Monsters() override = default;

	void MouseEvent(FLOAT timeElapsed);
	void KeyboardEvent(FLOAT timeElapsed);

	virtual void Update(FLOAT timeElapsed);
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) override; 

	void Move(XMFLOAT3 direction, FLOAT speed);  // �̵� ���� �߰�

	void SetCamera(const shared_ptr<Camera>& camera);
	shared_ptr<Camera> GetCamera() const { return m_camera; } 

	void SetScale(XMFLOAT3 scale);
	XMFLOAT3 GetScale() const;

	void SetAnimationClips(const std::vector<AnimationClip>& clips);
	void SetCurrentAnimation(const std::string& name);
	std::string GetCurrentAnimation() { return m_currentAnim; }
	void UploadBoneMatricesToShader(const std::vector<XMMATRIX>& boneTransforms, const ComPtr<ID3D12GraphicsCommandList>& commandList);

	// SRV ���� �Լ�
	void CreateBoneMatrixSRV(const ComPtr<ID3D12Device>& device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle);

	void SetMesh(const shared_ptr<MeshBase>& mesh)
	{
		m_meshes.clear();
		m_meshes.push_back(mesh);
	}

	void SetBoneOffsets(const unordered_map<int, XMMATRIX>& offsets) { m_boneOffsets = offsets; }
	void SetBoneNameToIndex(const unordered_map<string, int>& map) { m_boneNameToIndex = map; }
	void SetBoneHierarchy(const unordered_map<string, string>& boneHierarchy) { m_boneHierarchy = boneHierarchy; }
	void SetstaticNodeTransforms(const unordered_map<string, XMMATRIX>& staticNodeTransforms) { m_staticNodeTransforms = staticNodeTransforms;}
	void SetNodeNameToGlobalTransform(const unordered_map<string, XMMATRIX>& nodeNameToGlobalTransform) { m_nodeNameToLocalTransform = nodeNameToGlobalTransform;}
	void AddMesh(const shared_ptr<MeshBase>& mesh) { m_meshes.push_back(mesh); }

	void SetHP(int hp);
	bool IsDead() const { return m_isDead; }
	void ApplyDamage(float damage);
	bool IsParticleSpawned() const { return m_particleSpawned; }
	void MarkParticleSpawned() { m_particleSpawned = true; }
	void UpdateBoneMatrices(const ComPtr<ID3D12GraphicsCommandList>& commandList); // ���� �� ��� �ڵ� �и�
	//���� �ܰ��� ȣ��
	void RenderOutline(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;
	void PlayAnimationWithBlend(const std::string& newAnim, float blendDuration);
	int m_monNum;
	XMFLOAT3 m_prevPelvisPos = XMFLOAT3(0, 0, 0); // ���� �� �� �ʱ�ȭ �ʿ�
	bool m_hasPrevPelvisPos = false;
	BoundingBox AttackRange;
	bool isAttacking = false;
	float m_animTime = 0.f;
	shared_ptr<Camera> m_camera;

	FLOAT m_speed;

	// �ִϸ��̼� ���� ����
	std::unordered_map<std::string, AnimationClip> m_animationClips;
	std::string m_currentAnim = "Idle";
	std::string m_nextAnim = "";       // ������ �ִϸ��̼�
	float m_blendTime = 0.f;           // ���� ��� �ð�
	float m_blendDuration = 0.2f;      // ���� ���� �ð�
	bool m_isBlending = false;
	bool m_didDamageThisAnim = false;
	float m_prevAnimTime = -1.0f;
private:

	std::unordered_map<std::string, int> m_boneNameToIndex;
	ComPtr<ID3D12Resource> m_boneMatrixBuffer;             // GPU�� ����
	ComPtr<ID3D12Resource> m_boneMatrixUploadBuffer[2];       // ���ε�� ����
	D3D12_GPU_DESCRIPTOR_HANDLE m_boneMatrixSRV = {};      // ���̴����� ������ �ڵ�
	unordered_map<string, string> m_boneHierarchy;			//�� ��������
	unordered_map<string, XMMATRIX> m_staticNodeTransforms; 
	unordered_map<string, XMMATRIX> m_nodeNameToLocalTransform;	
	std::vector<std::shared_ptr<MeshBase>> m_meshes;
	unordered_map<int, XMMATRIX> m_boneOffsets;

	//�ִϸ��̼� ����ȭ��
	std::string m_prevAnimName = "";	
	std::pair<std::vector<XMMATRIX>, std::unordered_map<std::string, int>> m_cachedBoneTransforms;

	//���� ü�¹� ����
	shared_ptr<HealthBarObject> m_healthBar;
	float m_currentHP = 100.f;
	float m_maxHP = 100.f;

	//���� ����, ��ƼŬ ����
	bool m_isDead = false;
	bool m_particleSpawned = false;
	//-------------------------�ΰ��� ����-------------------------


};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class HealthBarObject : public GameObject
{
public:
	HealthBarObject(const ComPtr<ID3D12Device>& device);
	~HealthBarObject() override = default;

	void SetHP(float currentHP, float maxHP);

	virtual void Update(FLOAT timeElapsed) override; // ���� Object ��ӿ�
	void Update(FLOAT timeElapsed, const shared_ptr<Camera>& camera);
private:
	float m_currentHP = 100.f;
	float m_maxHP = 100.f;
	bool m_rotationFixed = false;
};