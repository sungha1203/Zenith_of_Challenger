#pragma once
#include "object.h"
#include "camera.h"
#include "Animation.h"
#include "d3dUtil.h"
#include "Shader.h"            // Shader를 shared_ptr로 보관하고 있음

class OtherPlayer : public GameObject
{
public:
	OtherPlayer(const ComPtr<ID3D12Device>& device); // 이거 추가해줘야 함!
	~OtherPlayer() override = default;

	virtual void Update(FLOAT timeElapsed) override;
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) override; // 추가

	void Move(XMFLOAT3 direction, FLOAT speed);  // 이동 로직 추가

	void SetScale(XMFLOAT3 scale);
	XMFLOAT3 GetScale() const;

	void SetAnimationClips(const std::vector<AnimationClip>& clips);
	void SetCurrentAnimation(const std::string& name);

	bool StartAnimation = false;

	//void UploadBoneMatricesToShader(const std::vector<XMMATRIX>& boneTransforms, std::unordered_map<std::string, int>animBoneIndex, const ComPtr<ID3D12GraphicsCommandList>& commandList);
	void UploadBoneMatricesToShader(const std::vector<XMMATRIX>& boneTransforms, const ComPtr<ID3D12GraphicsCommandList>& commandList);
	// SRV 생성 함수
	void CreateBoneMatrixSRV(const ComPtr<ID3D12Device>& device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle);

	void SetBoneOffsets(const unordered_map<int, XMMATRIX>& offsets) { m_boneOffsets = offsets; }
	void SetBoneNameToIndex(const unordered_map<string, int>& map) { m_boneNameToIndex = map; }
	void SetBoneHierarchy(const unordered_map<string, string>& boneHierarchy) { m_boneHierarchy = boneHierarchy; }
	void SetstaticNodeTransforms(const unordered_map<string, XMMATRIX>& staticNodeTransforms) { m_staticNodeTransforms = staticNodeTransforms; }
	unordered_map<string, XMMATRIX> m_staticNodeTransforms;
	void SetNodeNameToGlobalTransform(const unordered_map<string, XMMATRIX>& nodeNameToGlobalTransform) { m_nodeNameToLocalTransform = nodeNameToGlobalTransform; }
	unordered_map<string, XMMATRIX> m_nodeNameToLocalTransform;
	void UpdateBoneMatrices(const ComPtr<ID3D12GraphicsCommandList>& commandList); // other플레이어 뼈 행렬 코드 분리
	void AddMesh(const shared_ptr<MeshBase>& mesh) { m_meshes.push_back(mesh); }
	int m_id = -1;
	bool m_used = false;
	XMFLOAT3 m_position;
	std::string m_nextAnim = "";       // 블렌딩할 애니메이션
	float m_blendTime = 0.f;           // 블렌딩 경과 시간
	float m_blendDuration = 0.2f;      // 블렌딩 지속 시간
	bool m_isBlending = false;
	float m_angle; //방향각도 추가
	XMFLOAT3 oldPos = {0,0,0};
	std::string m_currentAnim = "Idle";

	int m_CurrentAnim = 0;

	// 애니메이션 상태 관리
	std::unordered_map<std::string, AnimationClip> m_animationClips;
	float m_animTime = 0.f;

	FLOAT m_speed;


	std::unordered_map<std::string, int> m_boneNameToIndex;
	ComPtr<ID3D12Resource> m_boneMatrixBuffer;             // GPU용 버퍼
	//ComPtr<ID3D12Resource> m_boneMatrixUploadBuffer;       // 업로드용 버퍼
	ComPtr<ID3D12Resource> m_boneMatrixUploadBuffer[2];       // 업로드용 버퍼
	D3D12_GPU_DESCRIPTOR_HANDLE m_boneMatrixSRV = {};      // 셰이더에서 접근할 핸들
	unordered_map<string, string> m_boneHierarchy;			//뼈 계층구조
	std::vector<std::shared_ptr<MeshBase>> m_meshes;
	unordered_map<int, XMMATRIX> m_boneOffsets;
private:

	//-------------------------인게임 정보-------------------------

};