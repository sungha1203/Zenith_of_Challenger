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
        D3D12_RESOURCE_STATE_GENERIC_READ); // ← 최종 상태 지정!

    char msg[128];
    sprintf_s(msg, "[Check] m_boneMatrixBuffer = %p\n", m_boneMatrixBuffer.Get());
    OutputDebugStringA(msg);
}


void Player::MouseEvent(FLOAT timeElapsed)
{
}

void Player::KeyboardEvent(FLOAT timeElapsed)
{
    if (!m_camera) return;

    XMFLOAT3 front{ m_camera->GetN() };
    front.y = 0.f;
    front = Vector3::Normalize(front);

    XMFLOAT3 right{ m_camera->GetU() };
    right.y = 0.f;
    right = Vector3::Normalize(right);

    XMFLOAT3 targetDirection{ 0.f, 0.f, 0.f };

    // 키 입력 상태 업데이트
    keyStates['W'] = (GetAsyncKeyState('W') & 0x8000);
    keyStates['S'] = (GetAsyncKeyState('S') & 0x8000);
    keyStates['A'] = (GetAsyncKeyState('A') & 0x8000);
    keyStates['D'] = (GetAsyncKeyState('D') & 0x8000);
    keyStates[VK_PRIOR] = (GetAsyncKeyState(VK_PRIOR) & 0x8000); // Page Up
    keyStates[VK_NEXT] = (GetAsyncKeyState(VK_NEXT) & 0x8000); // Page Down

    // 이동 방향 계산
    if (keyStates['W']) { targetDirection = Vector3::Add(targetDirection, front); }
    if (keyStates['S']) { targetDirection = Vector3::Add(targetDirection, Vector3::Negate(front)); }
    if (keyStates['A']) { targetDirection = Vector3::Add(targetDirection, Vector3::Negate(right)); }
    if (keyStates['D']) { targetDirection = Vector3::Add(targetDirection, right); }
    if (keyStates[VK_PRIOR]) { targetDirection = Vector3::Add(targetDirection, XMFLOAT3{ 0.f, 1.f, 0.f }); } // 위로 이동
    if (keyStates[VK_NEXT]) { targetDirection = Vector3::Add(targetDirection, XMFLOAT3{ 0.f, -1.f, 0.f }); } // 아래로 이동

    // 즉각적인 이동 (가속X, 바로 반응)
    if (!Vector3::IsZero(targetDirection))
    {
        targetDirection = Vector3::Normalize(targetDirection);
        velocity = Vector3::Mul(targetDirection, maxSpeed); // 최대 속도로 즉시 이동
    }
    else
    {
        // 즉시 멈추도록 설정
        velocity = { 0.f, 0.f, 0.f };
    }

    // 최종 이동
    Transform(Vector3::Mul(velocity, timeElapsed));
}

void Player::Update(FLOAT timeElapsed)
{
    if (m_camera) m_camera->UpdateEye(GetPosition());

    bool isMoving = keyStates['W'] || keyStates['A'] || keyStates['S'] || keyStates['D'];

    if (isMoving)
    {
        if (keyStates[VK_SHIFT] && m_animationClips.contains("Running"))
            SetCurrentAnimation("Running");
        else if (m_animationClips.contains("Walking"))
            SetCurrentAnimation("Walking");
    }
    else
    {
        if (m_animationClips.contains("M_C3FC_ModularMale_03"))
            SetCurrentAnimation("M_C3FC_ModularMale_03");
    }

    if (m_animationClips.contains(m_currentAnim))
    {
        const auto& clip = m_animationClips.at(m_currentAnim);
        m_animTime += timeElapsed * clip.ticksPerSecond;
        if (m_animTime > clip.duration)
            m_animTime = fmod(m_animTime, clip.duration);
    }

    {
        /////////////////////////여기다가 GetPosition()사용해서 float값 3개 가지고 오면 될거야

    }

    XMFLOAT3 pos = GetPosition();
    m_boundingBox.Center = XMFLOAT3{
        pos.x,
        pos.y + 0.5f,  // 중심이 피봇(발)보다 위로 가도록 보정
        pos.z
    };

    GameObject::Update(timeElapsed);
}


void Player::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
    // 애니메이션 본 행렬 업로드
    if (m_animationClips.contains(m_currentAnim))
    {
        const auto& clip = m_animationClips.at(m_currentAnim);
        float animTime = fmod(m_animTime, clip.duration);
        auto boneTransforms = clip.GetBoneTransforms(animTime);
        const_cast<Player*>(this)->UploadBoneMatricesToShader(boneTransforms, commandList);
    }

    // 본 행렬 StructuredBuffer 바인딩
    if (m_boneMatrixSRV.ptr != 0)
    {
        commandList->SetGraphicsRootDescriptorTable(RootParameter::BoneMatrix, m_boneMatrixSRV);
    }

    // 각 메시 렌더링
    for (const auto& mesh : m_meshes)
    {
        if (m_shader) m_shader->UpdateShaderVariable(commandList); // 셰이더 설정

        // 텍스처 바인딩
        if (m_texture) {
            m_texture->UpdateShaderVariable(commandList, m_textureIndex); // 보통 0번
        }
        // 머티리얼 바인딩
        if (m_material) m_material->UpdateShaderVariable(commandList);

        // 상속받은 GameObject의 상수버퍼 (world matrix 등)
        UpdateShaderVariable(commandList);

        // 메시 렌더링
        mesh->Render(commandList);
    }

    //와이어 프레임 렌더링
    if (m_drawBoundingBox && m_debugBoxMesh && m_debugLineShader)
    {
        m_debugLineShader->UpdateShaderVariable(commandList);

        // 디버깅 로그 추가 - 현재 worldMatrix를 콘솔에 출력
        const XMFLOAT4X4& mat = m_worldMatrix;
        char debugStr[256];
        sprintf_s(debugStr, "[AABB] worldMatrix pos: (%.2f, %.2f, %.2f)\n",
            mat._41, mat._42, mat._43);
        //OutputDebugStringA(debugStr);

        UpdateShaderVariable(commandList);

        m_debugBoxMesh->Render(commandList);
    }

}


void Player::UploadBoneMatricesToShader(const unordered_map<string, XMMATRIX>& boneTransforms, const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    const UINT MAX_BONES = 128;
    vector<XMMATRIX> finalMatrices(MAX_BONES, XMMatrixIdentity());

    for (const auto& [name, transform] : boneTransforms)
    {
        // 이름 -> 인덱스, 오프셋이 모두 존재하는 경우만 처리
        if (m_boneNameToIndex.contains(name) && m_boneOffsets.contains(name))
        {
            int index = m_boneNameToIndex[name];
            if (index < MAX_BONES)
            {
                // 핵심! 애니메이션 행렬 * 본 오프셋 행렬
                finalMatrices[index] = XMMatrixMultiply(transform, m_boneOffsets[name]);
            }
        }
    }

    // GPU에 업로드 (UploadBuffer → DefaultBuffer)
    D3D12_SUBRESOURCE_DATA subresourceData{};
    subresourceData.pData = finalMatrices.data();
    subresourceData.RowPitch = sizeof(XMMATRIX) * MAX_BONES;
    subresourceData.SlicePitch = subresourceData.RowPitch;

    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        m_boneMatrixBuffer.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));

    UpdateSubresources<1>(commandList.Get(), m_boneMatrixBuffer.Get(), m_boneMatrixUploadBuffer.Get(), 0, 0, 1, &subresourceData);

    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        m_boneMatrixBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
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
