#pragma once
#include "object.h"
#include "camera.h"
#include "Animation.h"
#include "d3dUtil.h"
#include "Shader.h"            // Shader를 shared_ptr로 보관하고 있음

class Player : public GameObject
{
public:
	Player(const ComPtr<ID3D12Device>& device); // 이거 추가해줘야 함!
	~Player() override = default;

	void MouseEvent(FLOAT timeElapsed);
	void KeyboardEvent(FLOAT timeElapsed);

	virtual void Update(FLOAT timeElapsed) override;
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const override; // 추가

	void Move(XMFLOAT3 direction, FLOAT speed);  // 이동 로직 추가
	bool isRunning = false;
	void SetCamera(const shared_ptr<Camera>& camera);
	shared_ptr<Camera> GetCamera() const { return m_camera; }  // 추가

	void SetScale(XMFLOAT3 scale);
	XMFLOAT3 GetScale() const;

	void SetAnimationClips(const std::vector<AnimationClip>& clips);
	void SetCurrentAnimation(const std::string& name);
	std::string& GetCurrentAnimation() { return m_currentAnim;}
	void UploadBoneMatricesToShader(const std::vector<XMMATRIX>& boneTransforms, const ComPtr<ID3D12GraphicsCommandList>& commandList);

	// SRV 생성 함수
	void CreateBoneMatrixSRV(const ComPtr<ID3D12Device>& device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle);

	void SetBoneOffsets(const unordered_map<int, XMMATRIX>& offsets) { m_boneOffsets = offsets; }
	void SetBoneNameToIndex(const unordered_map<string, int>& map) { m_boneNameToIndex = map; }
	void SetBoneHierarchy(const unordered_map<string, string>& boneHierarchy) { m_boneHierarchy = boneHierarchy; }
	void SetstaticNodeTransforms(const unordered_map<string, XMMATRIX>& staticNodeTransforms) { m_staticNodeTransforms = staticNodeTransforms; } 
	unordered_map<string, XMMATRIX> m_staticNodeTransforms;
	void UpdateBoneMatrices(const ComPtr<ID3D12GraphicsCommandList>& commandList); // 플레이어 뼈 행렬 코드 분리
	void SetNodeNameToGlobalTransform(const unordered_map<string, XMMATRIX>& nodeNameToGlobalTransform) { m_nodeNameToLocalTransform = nodeNameToGlobalTransform; } 
	unordered_map<string, XMMATRIX> m_nodeNameToLocalTransform;
	std::string m_nextAnim = "";       // 블렌딩할 애니메이션
	float m_blendTime = 0.f;           // 블렌딩 경과 시간
	float m_blendDuration = 0.2f;      // 블렌딩 지속 시간
	bool m_isBlending = false;

	void SetAttBoundingBox(const BoundingBox& box);
	void AddMesh(const shared_ptr<MeshBase>& mesh) { m_meshes.push_back(mesh); }
	int m_id;
	void PlayAnimationWithBlend(const std::string& newAnim, float blendDuration);
	bool isPunching=false;

private:
	shared_ptr<Camera> m_camera;
	//공격충돌체크 용
	BoundingBox m_AttboundingBox;
	FLOAT m_speed;

	// 애니메이션 상태 관리
	std::unordered_map<std::string, AnimationClip> m_animationClips;
	std::string m_currentAnim = "Idle";
	float m_animTime = 0.f;

	std::unordered_map<std::string, int> m_boneNameToIndex;
	ComPtr<ID3D12Resource> m_boneMatrixBuffer;             // GPU용 버퍼
	//ComPtr<ID3D12Resource> m_boneMatrixUploadBuffer;       // 업로드용 버퍼
	ComPtr<ID3D12Resource> m_boneMatrixUploadBuffer[2];       // 업로드용 버퍼
	D3D12_GPU_DESCRIPTOR_HANDLE m_boneMatrixSRV = {};      // 셰이더에서 접근할 핸들
	unordered_map<string, string> m_boneHierarchy;			//뼈 계층구조

	std::vector<std::shared_ptr<MeshBase>> m_meshes;
	unordered_map<int, XMMATRIX> m_boneOffsets;

	//-------------------------인게임 정보-------------------------

};