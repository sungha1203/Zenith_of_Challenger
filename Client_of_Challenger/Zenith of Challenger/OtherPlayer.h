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
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const override; // 추가

	void Move(XMFLOAT3 direction, FLOAT speed);  // 이동 로직 추가

	void SetScale(XMFLOAT3 scale);
	XMFLOAT3 GetScale() const;

	void SetAnimationClips(const std::vector<AnimationClip>& clips);
	void SetCurrentAnimation(const std::string& name);
	void UploadBoneMatricesToShader(const unordered_map<string, XMMATRIX>& boneTransforms, const ComPtr<ID3D12GraphicsCommandList>& commandList);

	// SRV 생성 함수
	void CreateBoneMatrixSRV(const ComPtr<ID3D12Device>& device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle);

	void SetBoneOffsets(const unordered_map<string, XMMATRIX>& offsets) { m_boneOffsets = offsets; }
	void SetBoneNameToIndex(const unordered_map<string, int>& map) { m_boneNameToIndex = map; }
	void AddMesh(const shared_ptr<MeshBase>& mesh) { m_meshes.push_back(mesh); }
	int m_id = -1;
	bool m_used = false;
	XMFLOAT3 m_position;

	float m_angle; //방향각도 추가
private:

	FLOAT m_speed;

	// 애니메이션 상태 관리
	std::unordered_map<std::string, AnimationClip> m_animationClips;
	std::string m_currentAnim = "Idle";
	float m_animTime = 0.f;

	std::unordered_map<std::string, int> m_boneNameToIndex;
	ComPtr<ID3D12Resource> m_boneMatrixBuffer;             // GPU용 버퍼
	ComPtr<ID3D12Resource> m_boneMatrixUploadBuffer;       // 업로드용 버퍼
	D3D12_GPU_DESCRIPTOR_HANDLE m_boneMatrixSRV = {};      // 셰이더에서 접근할 핸들


	std::vector<std::shared_ptr<MeshBase>> m_meshes;
	unordered_map<string, XMMATRIX> m_boneOffsets;

	//-------------------------인게임 정보-------------------------

};