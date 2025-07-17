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

	//d3dUtil::CreateDefaultBuffer(
	//	device.Get(),
	//	gGameFramework->GetCommandList().Get(),
	//	initBones.data(),
	//	bufferSize,
	//	m_boneMatrixBuffer,
	//	m_boneMatrixUploadBuffer,
	//	D3D12_RESOURCE_STATE_GENERIC_READ);
	// 본 버퍼 (GPU 상시 사용 대상)
	ComPtr<ID3D12Resource> dummyUploadBuffer;
	d3dUtil::CreateDefaultBuffer( 
		device.Get(), 
		gGameFramework->GetCommandList().Get(), 
		initBones.data(), 
		bufferSize, 
		m_boneMatrixBuffer,  
		/*업로드 버퍼*/ dummyUploadBuffer,
		D3D12_RESOURCE_STATE_GENERIC_READ); 

	// UploadBuffer만 2개 생성
	for (int i = 0; i < 2; ++i)
	{
		d3dUtil::CreateUploadBuffer( 
			device.Get(),
			bufferSize,
			m_boneMatrixUploadBuffer[i]); 
	}

	//체력바 생성
	m_healthBar = make_shared<HealthBarObject>(device);
	AttackRange.Extents= { 10.0f, 10.0f, 10.0f };
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
	if (m_isBlending)
	{
		m_blendTime += timeElapsed;

		if (m_blendTime >= m_blendDuration)
		{
			// 블렌드 완료
			m_currentAnim = m_nextAnim;
			m_animTime = 0.f;
			m_nextAnim.clear();
			m_isBlending = false;
		}
	
	}
	else
	{
		if (m_animationClips.contains(m_currentAnim))
		{
			const auto& clip = m_animationClips.at(m_currentAnim);
			m_animTime += timeElapsed * clip.ticksPerSecond;			
			while (m_animTime >= clip.duration)
				m_animTime -= clip.duration;		
		}		
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
		if(m_monNum==2|| m_monNum == 3)
		{
			SetRotationY(degrees+180);
			//SetRotationZ(90.f);
		}
		else if (m_monNum==0)
		{
			SetRotationY(degrees + 180.f);
			//SetRotationZ(180.f);
		}
		else if(m_monNum==1 ||m_monNum==4)
		{
			SetRotationY(degrees+180);
		}
		else
		{
			//SetRotationY(degrees + 90.f);
			//SetRotationZ(160.f);
		}


		//PlusRotationY(90.0f);
		//PlusRotationZ(90.0f);
	}


	GameObject::Update(timeElapsed);

	if (m_isDissolving)
	{
		m_dissolveTimer += timeElapsed;

		float t = m_dissolveTimer / m_dissolveDuration;
		t = std::clamp(t, 0.0f, 1.0f);
		SetDissolveAmount(t);

		OutputDebugStringA(("Boss dissolveAmount: " + std::to_string(t) + "\n").c_str());

		if (t >= 1.0f)
		{
			m_isDissolving = false;
			m_isDead = true;
			m_isActive = false; // 완전 제거
		}
	}


	if (m_healthBar)
	{
		m_healthBar->SetPosition(XMFLOAT3(m_position.x, m_position.y + 11.0f, m_position.z));
		m_healthBar->SetHP(m_currentHP, m_maxHP);
		m_healthBar->Update(timeElapsed, m_camera);
	}

}

void Monsters::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	if (m_isDead) return;
	bool isShadowPass = m_shader && m_shader->IsShadowShader();

	if (m_boneMatrixSRV.ptr != 0)
	{
		commandList->SetGraphicsRootDescriptorTable(RootParameter::MonsterBoneMatrix, m_boneMatrixSRV);
	}

	for (const auto& mesh : m_meshes)
	{
		m_shader->UpdateShaderVariable(commandList);

		if (!isShadowPass)
		{
			if (m_texture)  m_texture->UpdateShaderVariable(commandList, m_textureIndex);
			if (m_material) m_material->UpdateShaderVariable(commandList);
		}

		UpdateShaderVariable(commandList);

		mesh->Render(commandList);
		//와이어 프레임 렌더링
		if (!isShadowPass && m_drawBoundingBox && m_debugBoxMesh && m_debugLineShader)
		{
			m_debugLineShader->UpdateShaderVariable(commandList);
			m_debugBoxMesh->Render(commandList);
		}
	}

	if (!isShadowPass && m_healthBar)
	{
		m_HealthBarShader->UpdateShaderVariable(commandList);
		UpdateShaderVariable(commandList);
		m_healthBar->Render(commandList);
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
	//void* mappedData = nullptr;
	//CD3DX12_RANGE readRange(0, 0);
	//m_boneMatrixUploadBuffer->Map(0, &readRange, &mappedData);
	//memcpy(mappedData, finalMatrices.data(), sizeof(XMMATRIX) * finalMatrices.size());
	//m_boneMatrixUploadBuffer->Unmap(0, nullptr);
	//
	//commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
	//	m_boneMatrixBuffer.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));
	//
	//commandList->CopyResource(m_boneMatrixBuffer.Get(), m_boneMatrixUploadBuffer.Get());
	//
	//commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
	//	m_boneMatrixBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	// === UploadBuffer만 더블버퍼링 ===
	UINT frameIndex = gGameFramework->GetCurrentFrameIndex(); // 0 또는 1
	void* mappedData = nullptr;
	CD3DX12_RANGE readRange(0, 0);
	m_boneMatrixUploadBuffer[frameIndex]->Map(0, &readRange, &mappedData);
	memcpy(mappedData, finalMatrices.data(), sizeof(XMMATRIX) * MAX_BONES);
	m_boneMatrixUploadBuffer[frameIndex]->Unmap(0, nullptr);
	char debug[512];
	//sprintf_s(debug, "\n[디버그] BoneMatrixFrame 시작 (몬스터 이름: %s)\n", m_name.c_str()); // m_name은 몬스터 이름이라고 가정
	//OutputDebugStringA(debug);

	//for (int i = 0; i < 5; ++i)
	//{	
	//
	//	XMFLOAT4X4 mat;
	//	XMStoreFloat4x4(&mat, finalMatrices[i]);
	//
	//	sprintf_s(debug,
	//		"Bone[%02d]:\n"
	//		"  %.3f %.3f %.3f %.3f\n"
	//		"  %.3f %.3f %.3f %.3f\n"
	//		"  %.3f %.3f %.3f %.3f\n"
	//		"  %.3f %.3f %.3f %.3f\n",
	//		i,
	//		mat._11, mat._12, mat._13, mat._14,
	//		mat._21, mat._22, mat._23, mat._24,
	//		mat._31, mat._32, mat._33, mat._34,
	//		mat._41, mat._42, mat._43, mat._44);
	//	OutputDebugStringA(debug);
	//}

	// === 실제 m_boneMatrixBuffer에 복사 ===
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_boneMatrixBuffer.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));

	commandList->CopyResource(
		m_boneMatrixBuffer.Get(),
		m_boneMatrixUploadBuffer[frameIndex].Get());

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_boneMatrixBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
	//OutputDebugStringA("UploadBoneMatricesToShader 완료\n");

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

void Monsters::SetHP(int hp)
{
	m_currentHP = hp;
	if (m_healthBar) m_healthBar->SetHP(m_currentHP, m_maxHP);
	if (m_currentHP <= 0.f)
	{
		m_currentHP = 0.f;
		m_isDead = true;
	}
}

void Monsters::ApplyDamage(float damage)
{
	m_currentHP -= damage;
	if (m_currentHP <= 0.f && !m_isDead)
	{
		m_currentHP = 0.f;
		//m_isDead = true;
		StartDissolve(); // 디졸브 시작
	}
}

void Monsters::UpdateBoneMatrices(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	std::vector<XMMATRIX> boneTransforms;

	if (m_isBlending && m_animationClips.contains(m_currentAnim) && m_animationClips.contains(m_nextAnim))
	{
		// 현재 & 다음 애니메이션 정보
		const auto& fromClip = m_animationClips.at(m_currentAnim);
		const auto& toClip = m_animationClips.at(m_nextAnim);

		float fromTime = fmod(m_animTime, fromClip.duration);
		float toTime = (m_blendTime / m_blendDuration) * toClip.duration;

		// 각 본 행렬 계산
		auto fromBones = fromClip.GetBoneTransforms(fromTime, m_boneNameToIndex, m_boneHierarchy, m_boneOffsets, m_nodeNameToLocalTransform);
		auto toBones = toClip.GetBoneTransforms(toTime, m_boneNameToIndex, m_boneHierarchy, m_boneOffsets, m_nodeNameToLocalTransform);

		boneTransforms.resize(fromBones.size());
		float alpha = m_blendTime / m_blendDuration;

		for (size_t i = 0; i < boneTransforms.size(); ++i)
		{
			XMVECTOR scaleA, rotA, transA;
			XMVECTOR scaleB, rotB, transB;

			XMMatrixDecompose(&scaleA, &rotA, &transA, fromBones[i]);
			XMMatrixDecompose(&scaleB, &rotB, &transB, toBones[i]);

			XMVECTOR blendedScale = XMVectorLerp(scaleA, scaleB, alpha);
			XMVECTOR blendedRot = XMQuaternionSlerp(rotA, rotB, alpha);
			XMVECTOR blendedTrans = XMVectorLerp(transA, transB, alpha);

			boneTransforms[i] = XMMatrixAffineTransformation(blendedScale, XMVectorZero(), blendedRot, blendedTrans);
		}
	}
	else if (m_animationClips.contains(m_currentAnim))
	{
		const auto& clip = m_animationClips.at(m_currentAnim);
		float time = fmod(m_animTime, clip.duration);

		boneTransforms = clip.GetBoneTransforms(time, m_boneNameToIndex, m_boneHierarchy, m_boneOffsets, m_nodeNameToLocalTransform);		
	}


	if (!boneTransforms.empty())
	{
		UploadBoneMatricesToShader(boneTransforms, commandList);
	}
}

void Monsters::RenderOutline(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	if (m_isDead) return;
	if (!m_outlineShader) return;

	// 애니메이션 본 행렬 업로드
	if (m_animationClips.contains(m_currentAnim))
	{
		const auto& clip = m_animationClips.at(m_currentAnim);
		float animTime = fmod(m_animTime, clip.duration);
		auto boneTransforms = clip.GetBoneTransforms(animTime, m_boneNameToIndex, m_boneHierarchy, m_boneOffsets, m_nodeNameToLocalTransform);

		const_cast<Monsters*>(this)->UploadBoneMatricesToShader(boneTransforms, commandList);
	}

	// SRV 설정
	if (m_boneMatrixSRV.ptr != 0)
	{
		commandList->SetGraphicsRootDescriptorTable(RootParameter::MonsterBoneMatrix, m_boneMatrixSRV);
	}

	for (const auto& mesh : m_meshes)
	{
		m_outlineShader->UpdateShaderVariable(commandList);
		UpdateShaderVariable(commandList); // 월드행렬 전송
		mesh->Render(commandList);
	}
}

void Monsters::PlayAnimationWithBlend(const std::string& newAnim, float blendDuration)
{
	if (m_currentAnim == newAnim || !m_animationClips.contains(newAnim)) return;

	m_nextAnim = newAnim;
	m_blendDuration = blendDuration;
	m_blendTime = 0.f;
	m_isBlending = true;
}

void Monsters::StartDissolve()
{
	m_isDissolving = true;
	m_dissolveTimer = 0.0f;

	// 디졸브 기준축: Y축 (아래 → 위)
	SetDissolveAxis(Vector3::Normalize({ 0.f, 1.f, 0.f }));

	// 현재 위치를 기준점으로 설정
	SetDissolveOrigin(GetPosition());

	// 초기 디졸브 진행도
	SetDissolveAmount(0.f);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HealthBarObject::HealthBarObject(const ComPtr<ID3D12Device>& device) : GameObject(device)
{
	// 체력바용 얇은 사각형 생성 (Position + Normal + UV)
	vector<TextureVertex> vertices = {
		TextureVertex(XMFLOAT3(-2.5f, 0.0f, 0.0f), XMFLOAT3(0,1,0), XMFLOAT2(0,0)),
		TextureVertex(XMFLOAT3(2.5f, 0.0f, 0.0f), XMFLOAT3(0,1,0), XMFLOAT2(1,0)),
		TextureVertex(XMFLOAT3(-2.5f, 0.2f, 0.0f), XMFLOAT3(0,1,0), XMFLOAT2(0,1)),

		TextureVertex(XMFLOAT3(2.5f, 0.0f, 0.0f), XMFLOAT3(0,1,0), XMFLOAT2(1,0)),
		TextureVertex(XMFLOAT3(2.5f, 0.2f, 0.0f), XMFLOAT3(0,1,0), XMFLOAT2(1,1)),
		TextureVertex(XMFLOAT3(-2.5f, 0.2f, 0.0f), XMFLOAT3(0,1,0), XMFLOAT2(0,1))
	};

	auto deviceHandle = gGameFramework->GetDevice();
	auto commandList = gGameFramework->GetCommandList();
	m_mesh = make_shared<Mesh<TextureVertex>>(deviceHandle, commandList, vertices, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_baseColor = XMFLOAT4(1.f, 0.f, 0.f, 1.f); // 기본 빨간색
	m_useTexture = FALSE;
}

void HealthBarObject::SetHP(float currentHP, float maxHP)
{
	m_currentHP = currentHP;
	m_maxHP = maxHP;
}

void HealthBarObject::Update(FLOAT timeElapsed)
{
	Update(timeElapsed, nullptr);
}

void HealthBarObject::Update(FLOAT timeElapsed, const shared_ptr<Camera>& camera)
{
	float ratio = m_currentHP / m_maxHP;
	ratio = max(0.0f, min(1.0f, ratio));

	float xScale = m_isBoss ? ratio * 8.0f : ratio;
	float yScale = m_isBoss ? 5.0f : 1.0f;

	SetScale(XMFLOAT3(xScale, yScale, 1.f));

	if (ratio > 0.5f)
		m_baseColor = XMFLOAT4(1.f, 0.f, 0.f, 1.f);
	else if (ratio > 0.2f)
		m_baseColor = XMFLOAT4(1.f, 1.f, 0.f, 1.f);
	else
		m_baseColor = XMFLOAT4(0.f, 1.f, 0.f, 1.f);

	if (camera)
	{
		XMFLOAT3 pos = GetPosition();
		if (m_isBoss) pos.y += 30.0f;

		XMFLOAT3 cameraRight = camera->GetU();
		XMFLOAT3 cameraUp = camera->GetV();

		XMMATRIX world =
			XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z) *
			XMMATRIX(
				XMLoadFloat3(&cameraRight),
				XMLoadFloat3(&cameraUp),
				XMVectorSet(0.f, 0.f, 1.f, 0.f),
				XMVectorSet(pos.x, pos.y, pos.z, 1.f)
			);

		XMStoreFloat4x4(&m_worldMatrix, world);
	}
}