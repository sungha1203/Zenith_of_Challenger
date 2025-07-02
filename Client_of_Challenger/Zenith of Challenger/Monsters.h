#pragma once
#include "object.h"
#include "camera.h"
#include "Animation.h"
#include "d3dUtil.h"
#include "Shader.h"            // Shader를 shared_ptr로 보관하고 있음

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

	void Move(XMFLOAT3 direction, FLOAT speed);  // 이동 로직 추가

	void SetCamera(const shared_ptr<Camera>& camera);
	shared_ptr<Camera> GetCamera() const { return m_camera; } 

	void SetScale(XMFLOAT3 scale);
	XMFLOAT3 GetScale() const;

	void SetAnimationClips(const std::vector<AnimationClip>& clips);
	void SetCurrentAnimation(const std::string& name);
	std::string GetCurrentAnimation() { return m_currentAnim; }
	void UploadBoneMatricesToShader(const std::vector<XMMATRIX>& boneTransforms, const ComPtr<ID3D12GraphicsCommandList>& commandList);

	// SRV 생성 함수
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
	void UpdateBoneMatrices(const ComPtr<ID3D12GraphicsCommandList>& commandList); // 몬스터 뼈 행렬 코드 분리
	//몬스터 외곽선 호출
	void RenderOutline(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;
	void PlayAnimationWithBlend(const std::string& newAnim, float blendDuration);
	int m_monNum;
	XMFLOAT3 m_prevPelvisPos = XMFLOAT3(0, 0, 0); // 최초 한 번 초기화 필요
	bool m_hasPrevPelvisPos = false;
	BoundingBox AttackRange;
	bool isAttacking = false;
	float m_animTime = 0.f;
	shared_ptr<Camera> m_camera;

	FLOAT m_speed;

	// 애니메이션 상태 관리
	std::unordered_map<std::string, AnimationClip> m_animationClips;
	std::string m_currentAnim = "Idle";
	std::string m_nextAnim = "";       // 블렌딩할 애니메이션
	float m_blendTime = 0.f;           // 블렌딩 경과 시간
	float m_blendDuration = 0.2f;      // 블렌딩 지속 시간
	bool m_isBlending = false;
	bool m_didDamageThisAnim = false;
	float m_prevAnimTime = -1.0f;
private:

	std::unordered_map<std::string, int> m_boneNameToIndex;
	ComPtr<ID3D12Resource> m_boneMatrixBuffer;             // GPU용 버퍼
	ComPtr<ID3D12Resource> m_boneMatrixUploadBuffer[2];       // 업로드용 버퍼
	D3D12_GPU_DESCRIPTOR_HANDLE m_boneMatrixSRV = {};      // 셰이더에서 접근할 핸들
	unordered_map<string, string> m_boneHierarchy;			//뼈 계층구조
	unordered_map<string, XMMATRIX> m_staticNodeTransforms; 
	unordered_map<string, XMMATRIX> m_nodeNameToLocalTransform;	
	std::vector<std::shared_ptr<MeshBase>> m_meshes;
	unordered_map<int, XMMATRIX> m_boneOffsets;

	//애니메이션 최적화용
	std::string m_prevAnimName = "";	
	std::pair<std::vector<XMMATRIX>, std::unordered_map<std::string, int>> m_cachedBoneTransforms;

	//몬스터 체력바 관련
	shared_ptr<HealthBarObject> m_healthBar;
	float m_currentHP = 100.f;
	float m_maxHP = 100.f;

	//몬스터 죽음, 파티클 관련
	bool m_isDead = false;
	bool m_particleSpawned = false;
	//-------------------------인게임 정보-------------------------


};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class HealthBarObject : public GameObject
{
public:
	HealthBarObject(const ComPtr<ID3D12Device>& device);
	~HealthBarObject() override = default;

	void SetHP(float currentHP, float maxHP);

	virtual void Update(FLOAT timeElapsed) override; // 원래 Object 상속용
	void Update(FLOAT timeElapsed, const shared_ptr<Camera>& camera);
private:
	float m_currentHP = 100.f;
	float m_maxHP = 100.f;
	bool m_rotationFixed = false;
};