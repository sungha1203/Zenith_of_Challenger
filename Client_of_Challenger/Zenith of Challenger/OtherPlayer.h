#pragma once
#include "object.h"
#include "camera.h"
#include "Animation.h"
#include "d3dUtil.h"
#include "Shader.h"            // Shader�� shared_ptr�� �����ϰ� ����

class OtherPlayer : public GameObject
{
public:
	OtherPlayer(const ComPtr<ID3D12Device>& device); // �̰� �߰������ ��!
	~OtherPlayer() override = default;

	virtual void Update(FLOAT timeElapsed) override;
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) override; // �߰�

	void Move(XMFLOAT3 direction, FLOAT speed);  // �̵� ���� �߰�

	void SetScale(XMFLOAT3 scale);
	XMFLOAT3 GetScale() const;

	void SetAnimationClips(const std::vector<AnimationClip>& clips);
	void SetCurrentAnimation(const std::string& name);

	bool StartAnimation = false;

	//void UploadBoneMatricesToShader(const std::vector<XMMATRIX>& boneTransforms, std::unordered_map<std::string, int>animBoneIndex, const ComPtr<ID3D12GraphicsCommandList>& commandList);
	void UploadBoneMatricesToShader(const std::vector<XMMATRIX>& boneTransforms, const ComPtr<ID3D12GraphicsCommandList>& commandList);
	// SRV ���� �Լ�
	void CreateBoneMatrixSRV(const ComPtr<ID3D12Device>& device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle);

	void SetBoneOffsets(const unordered_map<int, XMMATRIX>& offsets) { m_boneOffsets = offsets; }
	void SetBoneNameToIndex(const unordered_map<string, int>& map) { m_boneNameToIndex = map; }
	void SetBoneHierarchy(const unordered_map<string, string>& boneHierarchy) { m_boneHierarchy = boneHierarchy; }
	void SetstaticNodeTransforms(const unordered_map<string, XMMATRIX>& staticNodeTransforms) { m_staticNodeTransforms = staticNodeTransforms; }
	unordered_map<string, XMMATRIX> m_staticNodeTransforms;
	void SetNodeNameToGlobalTransform(const unordered_map<string, XMMATRIX>& nodeNameToGlobalTransform) { m_nodeNameToLocalTransform = nodeNameToGlobalTransform; }
	unordered_map<string, XMMATRIX> m_nodeNameToLocalTransform;
	void UpdateBoneMatrices(const ComPtr<ID3D12GraphicsCommandList>& commandList); // other�÷��̾� �� ��� �ڵ� �и�
	void AddMesh(const shared_ptr<MeshBase>& mesh) { m_meshes.push_back(mesh); }
	int m_id = -1;
	bool m_used = false;
	XMFLOAT3 m_position;
	std::string m_nextAnim = "";       // ������ �ִϸ��̼�
	float m_blendTime = 0.f;           // ���� ��� �ð�
	float m_blendDuration = 0.2f;      // ���� ���� �ð�
	bool m_isBlending = false;
	float m_angle; //���Ⱒ�� �߰�
	XMFLOAT3 oldPos = {0,0,0};
	std::string m_currentAnim = "Idle";

	int m_CurrentAnim = 0;

	// �ִϸ��̼� ���� ����
	std::unordered_map<std::string, AnimationClip> m_animationClips;
	float m_animTime = 0.f;

	FLOAT m_speed;


	std::unordered_map<std::string, int> m_boneNameToIndex;
	ComPtr<ID3D12Resource> m_boneMatrixBuffer;             // GPU�� ����
	//ComPtr<ID3D12Resource> m_boneMatrixUploadBuffer;       // ���ε�� ����
	ComPtr<ID3D12Resource> m_boneMatrixUploadBuffer[2];       // ���ε�� ����
	D3D12_GPU_DESCRIPTOR_HANDLE m_boneMatrixSRV = {};      // ���̴����� ������ �ڵ�
	unordered_map<string, string> m_boneHierarchy;			//�� ��������
	std::vector<std::shared_ptr<MeshBase>> m_meshes;
	unordered_map<int, XMMATRIX> m_boneOffsets;
private:

	//-------------------------�ΰ��� ����-------------------------

};