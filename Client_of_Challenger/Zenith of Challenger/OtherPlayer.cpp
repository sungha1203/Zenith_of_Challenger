//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "OtherPlayer.h"
#include "stdafx.h"
#include "GameFramework.h"

// 키 입력 상태 저장용
//static unordered_map<int, bool> keyStates;
//XMFLOAT3 velocity = { 0.f, 0.f, 0.f };  // 현재 이동 속도
//const float maxSpeed = Settings::PlayerSpeed; // 최대 속도
//const float acceleration = 50.0f; // 즉각적인 반응을 위한 가속도 증가

OtherPlayer::OtherPlayer(const ComPtr<ID3D12Device>& device) :
    GameObject(device), m_speed{ Settings::PlayerSpeed }
{
    const UINT MAX_BONES = 128;
    UINT64 bufferSize = sizeof(XMMATRIX) * MAX_BONES;

    std::vector<XMMATRIX> initBones(MAX_BONES, XMMatrixIdentity());

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

void OtherPlayer::Update(FLOAT timeElapsed)
{


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
           
            if (m_animTime >= clip.duration)
                m_animTime -= clip.duration;
            
        }
        
    }

    
    if (m_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[0]) {
        m_position = gGameFramework->GetSceneManager()->GetCurrentScene()->otherpos[0];
        SetRotationY(gGameFramework->GetSceneManager()->GetCurrentScene()->otherangle[0]);
    }
    else if (m_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[1]) {
        m_position = gGameFramework->GetSceneManager()->GetCurrentScene()->otherpos[1];
        SetRotationY(gGameFramework->GetSceneManager()->GetCurrentScene()->otherangle[1]);
    }



    switch (m_CurrentAnim) {
    case 0:
        m_currentAnim = "Idle";
        break;
    case 1:
        m_currentAnim = "Walking";
        break;
    case 2:
        m_currentAnim = "Running";
        break;
    case 3:
        m_currentAnim = "Punch.001";
        break;
    case 7:
        m_currentAnim = "Slash";
        break;
    }



    SetPosition(m_position);

    XMFLOAT3 pos = GetPosition();
    m_boundingBox.Center = XMFLOAT3{
        pos.x,
        pos.y,  // 중심이 피봇(발)보다 위로 가도록 보정
        pos.z
    };

    GameObject::Update(timeElapsed);
}


void OtherPlayer::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{    

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

        m_debugBoxMesh->Render(commandList);
    }

}





void OtherPlayer::CreateBoneMatrixSRV(const ComPtr<ID3D12Device>& device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
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

void OtherPlayer::UpdateBoneMatrices(const ComPtr<ID3D12GraphicsCommandList>& commandList)
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

void OtherPlayer::Move(XMFLOAT3 direction, FLOAT speed)
{
    direction = Vector3::Normalize(direction);
    Transform(Vector3::Mul(direction, speed));
}

void OtherPlayer::SetScale(XMFLOAT3 scale)
{
    m_scale = scale;
    UpdateWorldMatrix();
}

XMFLOAT3 OtherPlayer::GetScale() const
{
    return m_scale;
}

void OtherPlayer::SetAnimationClips(const std::vector<AnimationClip>& clips)
{
    for (const auto& clip : clips)
        m_animationClips[clip.name] = clip;
}

void OtherPlayer::SetCurrentAnimation(const std::string& name)
{
    if (m_animationClips.contains(name))
    {
        m_currentAnim = name;
        m_animTime = 0.f;
    }
}

void OtherPlayer::UploadBoneMatricesToShader(const std::vector<XMMATRIX>& boneTransforms, const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    const UINT MAX_BONES = 128;
    std::vector<XMMATRIX> finalMatrices(MAX_BONES, XMMatrixIdentity());

    for (const auto& [boneName, vertexIndex] : m_boneNameToIndex)
    {
        finalMatrices[vertexIndex] = boneTransforms[vertexIndex];
    }
    for (int i = 0; i < finalMatrices.size(); ++i)
        finalMatrices[i] = XMMatrixTranspose(finalMatrices[i]);

    // === UploadBuffer만 더블버퍼링 ===
    UINT frameIndex = gGameFramework->GetCurrentFrameIndex(); // 0 또는 1
    void* mappedData = nullptr;
    CD3DX12_RANGE readRange(0, 0);
    m_boneMatrixUploadBuffer[frameIndex]->Map(0, &readRange, &mappedData);
    memcpy(mappedData, finalMatrices.data(), sizeof(XMMATRIX) * MAX_BONES);
    m_boneMatrixUploadBuffer[frameIndex]->Unmap(0, nullptr);
    char debug[512];
    
    // === 실제 m_boneMatrixBuffer에 복사 ===
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        m_boneMatrixBuffer.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));

    commandList->CopyResource(
        m_boneMatrixBuffer.Get(),
        m_boneMatrixUploadBuffer[frameIndex].Get());

    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        m_boneMatrixBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
}
