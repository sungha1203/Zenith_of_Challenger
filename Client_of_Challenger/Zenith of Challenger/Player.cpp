//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "player.h"
#include "stdafx.h"
#include "GameFramework.h"

// 키 입력 상태 저장용
static unordered_map<int, bool> keyStates;
XMFLOAT3 velocity = { 0.f, 0.f, 0.f };  // 현재 이동 속도
const float maxSpeed = Settings::PlayerSpeed; // 최대 속도
const float acceleration = 50.0f; // 즉각적인 반응을 위한 가속도 증가

Player::Player(const ComPtr<ID3D12Device>& device) :
    GameObject(device), m_speed{ Settings::PlayerSpeed }
{
    //const UINT MAX_BONES = 128;
    //UINT64 bufferSize = sizeof(XMMATRIX) * MAX_BONES;
    //
    //std::vector<XMMATRIX> initBones(MAX_BONES, XMMatrixIdentity());
    //
    //d3dUtil::CreateDefaultBuffer(
    //    device.Get(),
    //    gGameFramework->GetCommandList().Get(),
    //    initBones.data(),
    //    bufferSize,
    //    m_boneMatrixBuffer,
    //    m_boneMatrixUploadBuffer,
    //    D3D12_RESOURCE_STATE_GENERIC_READ); // ← 최종 상태 지정!
    //
    //char msg[128];
    //sprintf_s(msg, "[Check] m_boneMatrixBuffer = %p\n", m_boneMatrixBuffer.Get());
    //OutputDebugStringA(msg);
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
}


void Player::MouseEvent(FLOAT timeElapsed)
{
}

void Player::KeyboardEvent(FLOAT timeElapsed)
{
    auto gameScene = dynamic_cast<GameScene*>(gGameFramework->GetSceneManager()->GetCurrentScene().get());

    if (!gameScene->GetEndingSceneBool()) {

        if (!m_camera) return;

        bool isTPS = dynamic_pointer_cast<ThirdPersonCamera>(m_camera) != nullptr;
        bool isQuarter = dynamic_pointer_cast<QuarterViewCamera>(m_camera) != nullptr;

        XMFLOAT3 front, right;

        // 카메라 기준 방향
        if (isTPS || isQuarter)
        {
            front = m_camera->GetN(); front.y = 0.f;
            right = m_camera->GetU(); right.y = 0.f;
        }
        else
        {
            front = { 0.f, 0.f, 1.f };
            right = { 1.f, 0.f, 0.f };
        }

        front = Vector3::Normalize(front);
        right = Vector3::Normalize(right);

        XMFLOAT3 moveDirection{ 0.f, 0.f, 0.f };
        XMFLOAT3 faceDirection{ 0.f, 0.f, 0.f };

        // 키 입력 상태 업데이트
        if (!isPunching) {
            keyStates['W'] = (GetAsyncKeyState('W') & 0x8000);
            keyStates['S'] = (GetAsyncKeyState('S') & 0x8000);
            keyStates['A'] = (GetAsyncKeyState('A') & 0x8000);
            keyStates['D'] = (GetAsyncKeyState('D') & 0x8000);
        }
        else {
            keyStates['W'] = false;
            keyStates['S'] = false;
            keyStates['A'] = false;
            keyStates['D'] = false;
        }
        keyStates[VK_PRIOR] = (GetAsyncKeyState(VK_PRIOR) & 0x8000); // 위
        keyStates[VK_NEXT] = (GetAsyncKeyState(VK_NEXT) & 0x8000);   // 아래

        // 방향 계산
        if (keyStates['W']) moveDirection = Vector3::Add(moveDirection, front);
        if (keyStates['S']) moveDirection = Vector3::Add(moveDirection, Vector3::Negate(front));
        if (keyStates['A']) moveDirection = Vector3::Add(moveDirection, Vector3::Negate(right));
        if (keyStates['D']) moveDirection = Vector3::Add(moveDirection, right);
        if (keyStates[VK_PRIOR]) moveDirection = Vector3::Add(moveDirection, { 0.f, 1.f, 0.f });
        if (keyStates[VK_NEXT])  moveDirection = Vector3::Add(moveDirection, { 0.f, -1.f, 0.f });

        // 쿼터뷰 회전 방향 계산
        //if (isQuarter)
        //{
        if (keyStates['W']) faceDirection = Vector3::Add(faceDirection, front);
        if (keyStates['S']) faceDirection = Vector3::Add(faceDirection, Vector3::Negate(front));
        if (keyStates['A']) faceDirection = Vector3::Add(faceDirection, Vector3::Negate(right));
        if (keyStates['D']) faceDirection = Vector3::Add(faceDirection, right);
        // }

         // 속도 적용
        if (!Vector3::IsZero(moveDirection))
        {
            XMFLOAT3 normalized = Vector3::Normalize(moveDirection);
            if (isRunning)
                velocity = Vector3::Mul(normalized, maxSpeed + 20.0);
            else
                velocity = Vector3::Mul(normalized, maxSpeed);
        }
        else
        {
            velocity = { 0.f, 0.f, 0.f };
        }

        // 최종 이동
        XMFLOAT3 movement = Vector3::Mul(velocity, timeElapsed);
        if (!Vector3::IsZero(movement))
        {
            Transform(movement);
        }

        // 쿼터뷰 회전 적용
        if (/*isQuarter &&*/ !Vector3::IsZero(faceDirection))
        {
            faceDirection = Vector3::Normalize(faceDirection);
            float angle = atan2f(faceDirection.x, faceDirection.z);
            float degrees = XMConvertToDegrees(angle);
            SetRotationY(degrees + 180.f);
        }
    }
}



#pragma optimize( "", off )
void Player::Update(FLOAT timeElapsed)
{
    if (m_camera) m_camera->UpdateEye(GetPosition());

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
            m_animTime += timeElapsed * clip.ticksPerSecond * 2.0f;
            if (m_animTime > clip.duration)
            {
            	//m_animTime = fmod(m_animTime, clip.duration);
                m_animTime -= clip.duration;
            }
        }
    }

    bool isMoving = keyStates['W'] || keyStates['A'] || keyStates['S'] || keyStates['D'];

    if (isMoving && !isPunching)
    {
        if (GetAsyncKeyState(VK_SHIFT) && m_animationClips.contains("Running"))
        {
            m_currentAnim = "Running";
            isRunning = true;
            {
                CS_Packet_Animaition pkt;
                pkt.type = CS_PACKET_ANIMATION;
                pkt.animation = 2;
                pkt.size = sizeof(pkt);
                gGameFramework->GetClientNetwork()->SendPacket(reinterpret_cast<const char*>(&pkt), pkt.size);
            }
        }
        else if (m_animationClips.contains("Walking"))
        {
            /*SetCurrentAnimation("Walking");*/
            isRunning = false;
            {
                CS_Packet_Animaition pkt;
                pkt.type = CS_PACKET_ANIMATION;
                pkt.animation = 1;
                pkt.size = sizeof(pkt);
                gGameFramework->GetClientNetwork()->SendPacket(reinterpret_cast<const char*>(&pkt), pkt.size);
            }
            m_currentAnim = "Walking";
        }        
    }
    else if(isPunching)
    {
        //m_currentAnim = "Walking";
        XMFLOAT3 AttBoundingExtents = {3.0f,4.0f,3.0f};
        m_boundingBox.Extents = AttBoundingExtents;
        if (m_animTime > m_animationClips.at(m_currentAnim).duration - 5.0)
        {
            isPunching = false;
            isMoving = false;
            //SetCurrentAnimation("Idle");
            m_currentAnim = "Idle";
            m_boundingBox.Extents = { 1.0f, 4.0f, 1.0f };
            {
                CS_Packet_Animaition pkt;
                pkt.type = CS_PACKET_ANIMATION;
                pkt.animation = 0;
				pkt.size = sizeof(pkt);
				gGameFramework->GetClientNetwork()->SendPacket(reinterpret_cast<const char*>(&pkt), pkt.size);
			}
		}
	}
	else
	{
		if (m_animationClips.contains("Idle") && m_currentAnim != "Idle")
		{
			//  SetCurrentAnimation("Idle");
			m_currentAnim = "Idle";

			CS_Packet_Animaition pkt;
			pkt.type = CS_PACKET_ANIMATION;
			pkt.animation = 0;
			pkt.size = sizeof(pkt);
			gGameFramework->GetClientNetwork()->SendPacket(reinterpret_cast<const char*>(&pkt), pkt.size);

		}
	}


    {
        /////////////////////////여기다가 GetPosition()사용해서 float값 3개 가지고 오면 될거야

    }

    XMFLOAT3 pos = GetPosition();
    m_boundingBox.Center = XMFLOAT3{
        pos.x,
        pos.y,  // 중심이 피봇(발)보다 위로 가도록 보정
        pos.z
    };
    m_AttboundingBox.Center= XMFLOAT3{
        pos.x,
        pos.y,  // 중심이 피봇(발)보다 위로 가도록 보정
        pos.z
    };
    GameObject::Update(timeElapsed);
}
#pragma optimize( "", on )




void Player::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    bool isShadowPass = m_shader && m_shader->IsShadowShader();

    UpdateBoneMatrices(commandList);

    // 본 행렬 StructuredBuffer 바인딩
    if (m_boneMatrixSRV.ptr != 0)
    {
        commandList->SetGraphicsRootDescriptorTable(RootParameter::BoneMatrix, m_boneMatrixSRV);
    }

    // 각 메시 렌더링
    for (const auto& mesh : m_meshes)
    {
        if (m_shader) m_shader->UpdateShaderVariable(commandList); // 셰이더 설정

        if (!isShadowPass)
        {
            if (m_texture)  m_texture->UpdateShaderVariable(commandList, m_textureIndex);
            if (m_material) m_material->UpdateShaderVariable(commandList);
        }

        // 상속받은 GameObject의 상수버퍼 (world matrix 등)
        UpdateShaderVariable(commandList);

        // 메시 렌더링
        mesh->Render(commandList);
    }

    //와이어 프레임 렌더링
    if (!isShadowPass && m_drawBoundingBox && m_debugBoxMesh && m_debugLineShader)
    {
        m_debugLineShader->UpdateShaderVariable(commandList);
        m_debugBoxMesh->Render(commandList);
    }

}


void Player::UploadBoneMatricesToShader(const std::vector<XMMATRIX>& boneTransforms, const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    const UINT MAX_BONES = 128;
    std::vector<XMMATRIX> finalMatrices(MAX_BONES, XMMatrixIdentity());

    // 모든 본 인덱스를 안전하게 채움 (혹시 boneTransforms가 부족해도 T포즈 fallback)
    for (const auto& [boneName, vertexIndex] : m_boneNameToIndex)
    {
        if (vertexIndex < boneTransforms.size())
            finalMatrices[vertexIndex] = boneTransforms[vertexIndex];
        else {
            finalMatrices[vertexIndex] = XMMatrixIdentity();
            char dbg[256];
            sprintf_s(dbg, "[WARNING] Missing Bone[%d] (%s), fallback to Identity (T-pose)\n", vertexIndex, boneName.c_str());
            OutputDebugStringA(dbg);
        }
    }

    // HLSL column-major 방식으로 넘기기 위해 transpose
    for (auto& mat : finalMatrices)
        mat = XMMatrixTranspose(mat);

    // 프레임 인덱스에 따른 업로드 버퍼 선택
    UINT frameIndex = gGameFramework->GetCurrentFrameIndex();
    void* mappedData = nullptr;
    CD3DX12_RANGE readRange(0, 0);
    m_boneMatrixUploadBuffer[frameIndex]->Map(0, &readRange, &mappedData);
    memcpy(mappedData, finalMatrices.data(), sizeof(XMMATRIX) * MAX_BONES);
    m_boneMatrixUploadBuffer[frameIndex]->Unmap(0, nullptr);

    // GPU 리소스로 복사
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        m_boneMatrixBuffer.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));
    commandList->CopyResource(m_boneMatrixBuffer.Get(), m_boneMatrixUploadBuffer[frameIndex].Get());
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        m_boneMatrixBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

    // 디버그 로그
    char dbg[256];
    sprintf_s(dbg, "[Frame %d] AnimTime = %.4f, Anim = %s\n", frameIndex, m_animTime, m_currentAnim.c_str());
    //OutputDebugStringA(dbg);

    for (int i = 0; i < 5 && i < finalMatrices.size(); ++i) {
        const XMMATRIX& m = finalMatrices[i];
        sprintf_s(dbg, "Bone[%d]: %.2f %.2f %.2f %.2f\n", i,
            m.r[0].m128_f32[0], m.r[3].m128_f32[0], m.r[3].m128_f32[1], m.r[3].m128_f32[2]);
        //OutputDebugStringA(dbg);
    }
}






void Player::CreateBoneMatrixSRV(const ComPtr<ID3D12Device>& device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = 128; // MAX_BONES
    srvDesc.Buffer.StructureByteStride = sizeof(XMMATRIX);
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

    device->CreateShaderResourceView(m_boneMatrixBuffer.Get(), &srvDesc, cpuHandle);
    m_boneMatrixSRV = gpuHandle;
}

void Player::UpdateBoneMatrices(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    std::vector<XMMATRIX> boneTransforms;

    if (m_animationClips.contains(m_currentAnim))
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

void Player::SetAttBoundingBox(const BoundingBox& box)
{
    m_AttboundingBox = box;

    XMFLOAT3 c = box.Center;
    XMFLOAT3 e = box.Extents;

    XMFLOAT3 corners[8] = {
    {c.x - e.x, c.y - e.y, c.z - e.z},
    {c.x - e.x, c.y + e.y, c.z - e.z},
    {c.x + e.x, c.y + e.y, c.z - e.z},
    {c.x + e.x, c.y - e.y, c.z - e.z},
    {c.x - e.x, c.y - e.y, c.z + e.z},
    {c.x - e.x, c.y + e.y, c.z + e.z},
    {c.x + e.x, c.y + e.y, c.z + e.z},
    {c.x + e.x, c.y - e.y, c.z + e.z}
    };

    std::vector<UINT> indices = {
        // 앞면 (-z)
        0, 1, 2,
        0, 2, 3,

        // 뒷면 (+z)
        4, 6, 5,
        4, 7, 6,

        // 왼쪽면 (-x)
        0, 4, 5,
        0, 5, 1,

        // 오른쪽면 (+x)
        3, 2, 6,
        3, 6, 7,

        // 윗면 (+y)
        1, 5, 6,
        1, 6, 2,

        // 아랫면 (-y)
        0, 3, 7,
        0, 7, 4
    };

    vector<DebugVertex> lines;
    for (int i = 0; i < indices.size(); ++i)
        lines.emplace_back(corners[indices[i]]);

    m_AttdebugBoxMesh = make_shared<Mesh<DebugVertex>>(
        gGameFramework->GetDevice(), gGameFramework->GetCommandList(),
        lines, D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Player::PlayAnimationWithBlend(const std::string& newAnim, float blendDuration)
{
    if (m_currentAnim == newAnim || !m_animationClips.contains(newAnim)) return;

    m_nextAnim = newAnim;
    m_blendDuration = blendDuration;
    m_blendTime = 0.f;
    m_isBlending = true;
}

XMFLOAT3 Player::GetForward() const
{
    XMMATRIX world = XMLoadFloat4x4(&GetWorldMatrix());

    // 월드행렬의 z축 추출
    XMVECTOR forward = XMVector3Normalize(world.r[2]);
    forward = XMVectorNegate(forward);

    XMFLOAT3 dir;
    XMStoreFloat3(&dir, forward);

    return dir;
}
void Player::Move(XMFLOAT3 direction, FLOAT speed)
{
    direction = Vector3::Normalize(direction);
    Transform(Vector3::Mul(direction, speed));
}

void Player::SetCamera(const shared_ptr<Camera>& camera)
{
    m_camera = camera;
}

void Player::SetScale(XMFLOAT3 scale)
{
    m_scale = scale;
    UpdateWorldMatrix();
}

XMFLOAT3 Player::GetScale() const
{
    return m_scale;
}

void Player::SetAnimationClips(const std::vector<AnimationClip>& clips)
{
    for (const auto& clip : clips)
        m_animationClips[clip.name] = clip;
}

void Player::SetCurrentAnimation(const std::string& name)
{
    if (m_animationClips.contains(name))
    {
        m_currentAnim = name;
        m_animTime = 0.f;
    }
}