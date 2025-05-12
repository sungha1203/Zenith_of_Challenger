//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "OtherPlayer.h"
#include "stdafx.h"
#include "GameFramework.h"

// Ű �Է� ���� �����
//static unordered_map<int, bool> keyStates;
//XMFLOAT3 velocity = { 0.f, 0.f, 0.f };  // ���� �̵� �ӵ�
//const float maxSpeed = Settings::PlayerSpeed; // �ִ� �ӵ�
//const float acceleration = 50.0f; // �ﰢ���� ������ ���� ���ӵ� ����

OtherPlayer::OtherPlayer(const ComPtr<ID3D12Device>& device) :
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
   //    D3D12_RESOURCE_STATE_GENERIC_READ); // �� ���� ���� ����!
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
    // �� ���� (GPU ��� ��� ���)
    ComPtr<ID3D12Resource> dummyUploadBuffer;
    d3dUtil::CreateDefaultBuffer(
        device.Get(),
        gGameFramework->GetCommandList().Get(),
        initBones.data(),
        bufferSize,
        m_boneMatrixBuffer,
        /*���ε� ����*/ dummyUploadBuffer,
        D3D12_RESOURCE_STATE_GENERIC_READ);

    // UploadBuffer�� 2�� ����
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

    //bool isMoving = keyStates['W'] || keyStates['A'] || keyStates['S'] || keyStates['D'];
    //
    //if (isMoving)
    //{
    //    if (keyStates[VK_SHIFT] && m_animationClips.contains("Running"))
    //        SetCurrentAnimation("Running");
    //    else if (m_animationClips.contains("Walking"))
    //        SetCurrentAnimation("Walking");
    //}
    //else
    //{
    //    if (m_animationClips.contains("M_C3FC_ModularMale_03"))
    //        SetCurrentAnimation("M_C3FC_ModularMale_03");
    //}

    if (m_isBlending)
    {
        m_blendTime += timeElapsed;

        if (m_blendTime >= m_blendDuration)
        {
            // ���� �Ϸ�
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

    //if (m_animationClips.contains(m_currentAnim))
    //{
    //    const auto& clip = m_animationClips.at(m_currentAnim);
    //    m_animTime += timeElapsed * clip.ticksPerSecond;
    //    if (m_animTime > clip.duration)
    //        m_animTime = fmod(m_animTime, clip.duration);
    //}
    
    if (m_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[0]) {
        m_position = gGameFramework->GetSceneManager()->GetCurrentScene()->otherpos[0];
        SetRotationY(gGameFramework->GetSceneManager()->GetCurrentScene()->otherangle[0]);
    }
    else if (m_id == gGameFramework->GetSceneManager()->GetCurrentScene()->otherid[1]) {
        m_position = gGameFramework->GetSceneManager()->GetCurrentScene()->otherpos[1];
        SetRotationY(gGameFramework->GetSceneManager()->GetCurrentScene()->otherangle[1]);
    }


    //if (oldPos.x == m_position.x && oldPos.y == m_position.y && oldPos.z == m_position.z)
    //{
    //    if(m_currentAnim!="Punch.001")
    //    m_currentAnim = "Idle";
    //    else
    //    {
    //        if (m_animTime > m_animationClips.at(m_currentAnim).duration - 1.0)
    //        {                
    //            SetCurrentAnimation("Idle");
    //        }
    //    }
    //}
    //else
    //{
    //    oldPos = m_position;
    //    m_currentAnim = "Walking";
    //}
    float dx = m_position.x - oldPos.x;
    float dy = m_position.y - oldPos.y;
    float dz = m_position.z - oldPos.z;

    float distSq = dx * dx + dy * dy + dz * dz;
    const float moveThresholdSq = 0.0001f; // �ʹ� ������ �� ��

    if (distSq < moveThresholdSq)
    {
        if (m_currentAnim != "Punch.001")
        {
            m_currentAnim = "Idle";
        }
        else
        {
            if (m_animTime > m_animationClips.at(m_currentAnim).duration - 1.0f)
            {
                SetCurrentAnimation("Idle");
            }
        }
    }
    else
    {
        oldPos = m_position;
        m_currentAnim = "Walking";
    }
    SetPosition(m_position);

    XMFLOAT3 pos = GetPosition();
    m_boundingBox.Center = XMFLOAT3{
        pos.x,
        pos.y,  // �߽��� �Ǻ�(��)���� ���� ������ ����
        pos.z
    };

    GameObject::Update(timeElapsed);
}


void OtherPlayer::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
    //// �ִϸ��̼� �� ��� ���ε�
    //if (m_animationClips.contains(m_currentAnim))
    //{
    //    const auto& clip = m_animationClips.at(m_currentAnim);
    //    float animTime = fmod(m_animTime, clip.duration);
    //    auto [boneTransforms, animBoneIndex] = clip.GetBoneTransforms(animTime, m_boneNameToIndex, m_boneHierarchy,m_staticNodeTransforms);
    //    const_cast<OtherPlayer*>(this)->UploadBoneMatricesToShader(boneTransforms, animBoneIndex, commandList);
    //}

    // �� ��� StructuredBuffer ���ε�
    if (m_boneMatrixSRV.ptr != 0)
    {
        commandList->SetGraphicsRootDescriptorTable(RootParameter::BoneMatrix, m_boneMatrixSRV);
    }

    // �� �޽� ������
    for (const auto& mesh : m_meshes)
    {
        if (m_shader) m_shader->UpdateShaderVariable(commandList); // ���̴� ����

        // �ؽ�ó ���ε�
        if (m_texture) {
            m_texture->UpdateShaderVariable(commandList, m_textureIndex); // ���� 0��
        }
        // ��Ƽ���� ���ε�
        if (m_material) m_material->UpdateShaderVariable(commandList);

        // ��ӹ��� GameObject�� ������� (world matrix ��)
        UpdateShaderVariable(commandList);

        // �޽� ������
        mesh->Render(commandList);
    }

    //���̾� ������ ������
    if (m_drawBoundingBox && m_debugBoxMesh && m_debugLineShader)
    {
        m_debugLineShader->UpdateShaderVariable(commandList);

        m_debugBoxMesh->Render(commandList);
    }

}


//void OtherPlayer::UploadBoneMatricesToShader(const std::vector<XMMATRIX>& boneTransforms, std::unordered_map<std::string, int>animBoneIndex, const ComPtr<ID3D12GraphicsCommandList>& commandList)
//{
//    //const UINT MAX_BONES = 128;
//    //vector<XMMATRIX> finalMatrices(MAX_BONES, XMMatrixIdentity());
//
//    //size_t count = std::min<size_t>(boneTransforms.size(), MAX_BONES);
//
//    //for (size_t i = 0; i < count; ++i)
//    //{
//    //    // �� offset�� �����Ѵٸ� ������
//    //    if (m_boneOffsets.contains(i))
//    //    {
//    //        finalMatrices[i] = XMMatrixMultiply(boneTransforms[i], m_boneOffsets.at(i));
//    //    }
//    //    else
//    //    {
//    //        finalMatrices[i] = boneTransforms[i]; // fallback
//    //    }
//    //}
//    const UINT MAX_BONES = boneTransforms.size();
//    vector<XMMATRIX> finalMatrices(MAX_BONES, XMMatrixIdentity());
//
//    size_t count = std::min<size_t>(boneTransforms.size(), MAX_BONES);
//
//    //for (size_t i = 0; i < count; ++i)
//    //{
//    //    // �� offset�� �����Ѵٸ� ������
//    //    if (m_boneOffsets.contains(i))
//    //    {
//    //        finalMatrices[i] = XMMatrixMultiply(boneTransforms[i], m_boneOffsets.at(i));
//    //        //finalMatrices[i] = XMMatrixMultiply(m_boneOffsets.at(i), boneTransforms[i]);
//    //    }
//    //    else
//    //    {
//    //        finalMatrices[i] = boneTransforms[i]; // fallback
//    //    }
//    //}
//    for (const auto& [boneName, vertexIndex] : m_boneNameToIndex)
//    {
//        // animBoneIndex: boneName �� i (�ִϸ��̼� transform�� �ε���)
//        if (animBoneIndex.contains(boneName) && m_boneOffsets.contains(vertexIndex))
//        {
//            int animIndex = animBoneIndex.at(boneName);
//            finalMatrices[vertexIndex] = XMMatrixMultiply(boneTransforms[animIndex], m_boneOffsets.at(vertexIndex));
//        }
//    }
//    // GPU�� ���ε� (UploadBuffer �� DefaultBuffer)
//    D3D12_SUBRESOURCE_DATA subresourceData{};
//    subresourceData.pData = finalMatrices.data();
//    subresourceData.RowPitch = sizeof(XMMATRIX) * MAX_BONES;
//    subresourceData.SlicePitch = subresourceData.RowPitch;
//
//    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
//        m_boneMatrixBuffer.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));
//
//    UpdateSubresources<1>(commandList.Get(), m_boneMatrixBuffer.Get(), m_boneMatrixUploadBuffer.Get(), 0, 0, 1, &subresourceData);
//
//    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
//        m_boneMatrixBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
//}





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

    //if (m_isBlending && m_animationClips.contains(m_currentAnim) && m_animationClips.contains(m_nextAnim))
    //{
    //    // ���� & ���� �ִϸ��̼� ����
    //    const auto& fromClip = m_animationClips.at(m_currentAnim);
    //    const auto& toClip = m_animationClips.at(m_nextAnim);
    //
    //    float fromTime = fmod(m_animTime, fromClip.duration);
    //    float toTime = (m_blendTime / m_blendDuration) * toClip.duration;
    //
    //    // �� �� ��� ���
    //    auto fromBones = fromClip.GetBoneTransforms(fromTime, m_boneNameToIndex, m_boneHierarchy, m_boneOffsets, m_nodeNameToLocalTransform);
    //    auto toBones = toClip.GetBoneTransforms(toTime, m_boneNameToIndex, m_boneHierarchy, m_boneOffsets, m_nodeNameToLocalTransform);
    //
    //    boneTransforms.resize(fromBones.size());
    //    float alpha = m_blendTime / m_blendDuration;
    //
    //    for (size_t i = 0; i < boneTransforms.size(); ++i)
    //    {
    //        XMVECTOR scaleA, rotA, transA;
    //        XMVECTOR scaleB, rotB, transB;
    //
    //        XMMatrixDecompose(&scaleA, &rotA, &transA, fromBones[i]);
    //        XMMatrixDecompose(&scaleB, &rotB, &transB, toBones[i]);
    //
    //        XMVECTOR blendedScale = XMVectorLerp(scaleA, scaleB, alpha);
    //        XMVECTOR blendedRot = XMQuaternionSlerp(rotA, rotB, alpha);
    //        XMVECTOR blendedTrans = XMVectorLerp(transA, transB, alpha);
    //
    //        boneTransforms[i] = XMMatrixAffineTransformation(blendedScale, XMVectorZero(), blendedRot, blendedTrans);
    //    }
    //}
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

    // ===== GPU ���� =====
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

    // === UploadBuffer�� ������۸� ===
    UINT frameIndex = gGameFramework->GetCurrentFrameIndex(); // 0 �Ǵ� 1
    void* mappedData = nullptr;
    CD3DX12_RANGE readRange(0, 0);
    m_boneMatrixUploadBuffer[frameIndex]->Map(0, &readRange, &mappedData);
    memcpy(mappedData, finalMatrices.data(), sizeof(XMMATRIX) * MAX_BONES);
    m_boneMatrixUploadBuffer[frameIndex]->Unmap(0, nullptr);
    char debug[512];
    //sprintf_s(debug, "\n[�����] BoneMatrixFrame ���� (���� �̸�: %s)\n", m_name.c_str()); // m_name�� ���� �̸��̶�� ����
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
    // === ���� m_boneMatrixBuffer�� ���� ===
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        m_boneMatrixBuffer.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));

    commandList->CopyResource(
        m_boneMatrixBuffer.Get(),
        m_boneMatrixUploadBuffer[frameIndex].Get());

    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        m_boneMatrixBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
}
