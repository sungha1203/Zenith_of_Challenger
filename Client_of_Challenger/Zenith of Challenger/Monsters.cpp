//-----------------------------------------------------------------------------
// File: Monsters.cpp
//-----------------------------------------------------------------------------
#include "Monsters.h"
#include "GameFramework.h"
#include<pix3.h>

static unordered_map<int, bool> keyStates;
XMFLOAT3 monsterVelocity = { 0.f, 0.f, 0.f };

Monsters::Monsters(const ComPtr<ID3D12Device>& device) : GameObject(device), m_speed(1.0f)
{
	const UINT MAX_BONES = 128;
	UINT64 bufferSize = sizeof(XMMATRIX) * MAX_BONES;

	std::vector<XMMATRIX> initBones(MAX_BONES, XMMatrixIdentity());

	d3dUtil::CreateDefaultBuffer(
		device.Get(),
		gGameFramework->GetCommandList().Get(),
		initBones.data(),
		bufferSize,
		m_boneMatrixBuffer,
		m_boneMatrixUploadBuffer,
		D3D12_RESOURCE_STATE_GENERIC_READ);
}

void Monsters::MouseEvent(FLOAT timeElapsed)
{
	// 필요 시 작성
}

void Monsters::KeyboardEvent(FLOAT timeElapsed)
{
	// AI 몬스터는 직접 입력받지 않음
}

void Monsters::Update(FLOAT timeElapsed)
{
	// 추후 AI나 타겟 추적 등으로 확장 가능

	// 애니메이션 갱신
	if (m_animationClips.contains(m_currentAnim))
	{
		const auto& clip = m_animationClips.at(m_currentAnim);
		m_animTime += timeElapsed * clip.ticksPerSecond;
		if (m_animTime > clip.duration)
			m_animTime = fmod(m_animTime, clip.duration);
	}


	// [1] 플레이어 위치 받아오기
	if (!gGameFramework->GetPlayer()) return;

	XMFLOAT3 targetPos = gGameFramework->GetPlayer()->GetPosition();
	XMFLOAT3 myPos = GetPosition();

	// [2] 방향 벡터 계산
	XMFLOAT3 toPlayer = {
		targetPos.x - myPos.x,
		0.f, // Y축 회전이므로 높이 무시
		targetPos.z - myPos.z
	};

	if (!Vector3::IsZero(toPlayer))
	{
		toPlayer = Vector3::Normalize(toPlayer);

		// [3] 회전 각도 계산 (Z 기준)
		float angle = atan2f(toPlayer.x, toPlayer.z); // x/z
		float degrees = XMConvertToDegrees(angle);

		// [4] 몬스터가 플레이어를 향하도록 Y축 회전
		
		SetRotationY(degrees + 90.f);
		SetRotationZ(90.0f);
		//PlusRotationY(90.0f);
		//PlusRotationZ(90.0f);
	}


	GameObject::Update(timeElapsed);
}

void Monsters::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	if (m_animationClips.contains(m_currentAnim))
	{
		const auto& clip = m_animationClips.at(m_currentAnim);
		float animTime = fmod(m_animTime, clip.duration);
	
		auto boneTransforms = clip.GetBoneTransforms(animTime, m_boneNameToIndex, m_boneHierarchy, m_boneOffsets, m_nodeNameToGlobalTransform);
		const_cast<Monsters*>(this)->UploadBoneMatricesToShader(boneTransforms, commandList);
	}
	
	if (m_boneMatrixSRV.ptr != 0)
	{
		commandList->SetGraphicsRootDescriptorTable(RootParameter::MonsterBoneMatrix, m_boneMatrixSRV);
	}

	for (const auto& mesh : m_meshes)
	{
		m_shader->UpdateShaderVariable(commandList);

		if (m_texture)
			m_texture->UpdateShaderVariable(commandList, m_textureIndex);

		if (m_material)
			m_material->UpdateShaderVariable(commandList);

		UpdateShaderVariable(commandList);
		mesh->Render(commandList);
	}

	//와이어 프레임 렌더링
	if (m_drawBoundingBox && m_debugBoxMesh && m_debugLineShader)
	{
		m_debugLineShader->UpdateShaderVariable(commandList);

		m_debugBoxMesh->Render(commandList);
	}
}

void Monsters::Move(XMFLOAT3 direction, FLOAT speed)
{
	direction = Vector3::Normalize(direction);
	Transform(Vector3::Mul(direction, speed));
}

void Monsters::SetCamera(const shared_ptr<Camera>& camera)
{
	m_camera = camera;
}

void Monsters::SetScale(XMFLOAT3 scale)
{
	m_scale = scale;
	UpdateWorldMatrix();
}

XMFLOAT3 Monsters::GetScale() const
{
	return m_scale;
}

void Monsters::SetAnimationClips(const std::vector<AnimationClip>& clips)
{
	for (const auto& clip : clips)
		m_animationClips[clip.name] = clip;
}

void Monsters::SetCurrentAnimation(const std::string& name)
{
	if (m_animationClips.contains(name))
	{
		m_currentAnim = name;
		m_animTime = 0.f;
	}
}

void Monsters::UploadBoneMatricesToShader(const std::vector<XMMATRIX>& boneTransforms, const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	const UINT MAX_BONES = 128;
	std::vector<XMMATRIX> finalMatrices(MAX_BONES, XMMatrixIdentity());

	for (const auto& [boneName, vertexIndex] : m_boneNameToIndex)
	{
		finalMatrices[vertexIndex] = boneTransforms[vertexIndex];
	}
	for (int i = 0; i < finalMatrices.size(); ++i)
		finalMatrices[i] = XMMatrixTranspose(finalMatrices[i]);

	// ===== GPU 복사 =====
	PIXBeginEvent(0, "본 행렬 업로드");

	void* mappedData = nullptr;
	CD3DX12_RANGE readRange(0, 0);
	m_boneMatrixUploadBuffer->Map(0, &readRange, &mappedData);
	memcpy(mappedData, finalMatrices.data(), sizeof(XMMATRIX) * finalMatrices.size());
	m_boneMatrixUploadBuffer->Unmap(0, nullptr);

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_boneMatrixBuffer.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));

	commandList->CopyResource(m_boneMatrixBuffer.Get(), m_boneMatrixUploadBuffer.Get());

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_boneMatrixBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	PIXEndEvent();
}
void Monsters::CreateBoneMatrixSRV(const ComPtr<ID3D12Device>& device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = m_boneNameToIndex.size();
	srvDesc.Buffer.StructureByteStride = sizeof(XMMATRIX);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	device->CreateShaderResourceView(m_boneMatrixBuffer.Get(), &srvDesc, cpuHandle);
	m_boneMatrixSRV = gpuHandle;
}
