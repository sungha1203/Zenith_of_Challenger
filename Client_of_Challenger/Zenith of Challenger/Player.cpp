//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "player.h"
#include "stdafx.h"
#include "GameFramework.h"

// Ű �Է� ���� �����
static unordered_map<int, bool> keyStates;
XMFLOAT3 velocity = { 0.f, 0.f, 0.f };  // ���� �̵� �ӵ�
const float maxSpeed = Settings::PlayerSpeed; // �ִ� �ӵ�
const float acceleration = 50.0f; // �ﰢ���� ������ ���� ���ӵ� ����

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


void Player::MouseEvent(FLOAT timeElapsed)
{
}

void Player::KeyboardEvent(FLOAT timeElapsed)
{
    if (!m_camera) return;

    bool isTPS = dynamic_pointer_cast<ThirdPersonCamera>(m_camera) != nullptr;
    bool isQuarter = dynamic_pointer_cast<QuarterViewCamera>(m_camera) != nullptr;

    XMFLOAT3 front, right;

    // TPS�� ���ͺ� �� �� ī�޶� ���� �������� ������
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

    // Ű �Է� ���� ������Ʈ
    keyStates['W'] = (GetAsyncKeyState('W') & 0x8000);
    keyStates['S'] = (GetAsyncKeyState('S') & 0x8000);
    keyStates['A'] = (GetAsyncKeyState('A') & 0x8000);
    keyStates['D'] = (GetAsyncKeyState('D') & 0x8000);
    keyStates[VK_PRIOR] = (GetAsyncKeyState(VK_PRIOR) & 0x8000); // ��
    keyStates[VK_NEXT] = (GetAsyncKeyState(VK_NEXT) & 0x8000);   // �Ʒ�

    // �̵� ���� ���
    if (keyStates['W']) moveDirection = Vector3::Add(moveDirection, front);
    if (keyStates['S']) moveDirection = Vector3::Add(moveDirection, Vector3::Negate(front));
    if (keyStates['A']) moveDirection = Vector3::Add(moveDirection, Vector3::Negate(right));
    if (keyStates['D']) moveDirection = Vector3::Add(moveDirection, right);
    if (keyStates[VK_PRIOR]) moveDirection = Vector3::Add(moveDirection, { 0.f, 1.f, 0.f });
    if (keyStates[VK_NEXT]) moveDirection = Vector3::Add(moveDirection, { 0.f, -1.f, 0.f });

    // ���ͺ�� ȸ���� ���� ó��
    if (isQuarter)
    {
        if (keyStates['W']) faceDirection = Vector3::Add(faceDirection, front);
        if (keyStates['S']) faceDirection = Vector3::Add(faceDirection, Vector3::Negate(front));
        if (keyStates['A']) faceDirection = Vector3::Add(faceDirection, Vector3::Negate(right));
        if (keyStates['D']) faceDirection = Vector3::Add(faceDirection, right);
    }

    // �ӵ� ����
    if (!Vector3::IsZero(moveDirection))
    {
        XMFLOAT3 normalized = Vector3::Normalize(moveDirection);
        velocity = Vector3::Mul(normalized, maxSpeed);
    }
    else
    {
        velocity = { 0.f, 0.f, 0.f };
    }

    // ���� �̵�
    XMFLOAT3 movement = Vector3::Mul(velocity, timeElapsed);
    if (!Vector3::IsZero(movement))
    {
        Transform(movement);
    }

    // ���ͺ� ȸ��
    if (isQuarter && !Vector3::IsZero(faceDirection))
    {
        faceDirection = Vector3::Normalize(faceDirection);
        float angle = atan2f(faceDirection.x, faceDirection.z);
        float degrees = XMConvertToDegrees(angle);
        SetRotationY(degrees + 180.f); // Z-�� ���� ��� +180��
    }
}



void Player::Update(FLOAT timeElapsed)
{
    if (m_camera) m_camera->UpdateEye(GetPosition());

    // �ִϸ��̼� ����
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
            //if (m_animTime > clip.duration)
            //{
            //	m_animTime = fmod(m_animTime, clip.duration);
            //	//m_animTime = 0.0f;
            //}
            while (m_animTime >= clip.duration)
                m_animTime -= clip.duration;
            //char buffer[128];
            //sprintf_s(buffer, "AnimTime: %.4f\n", m_animTime);
            //OutputDebugStringA(buffer);
        }
        //if (m_animationClips.contains(m_currentAnim))
        //{
        //	const auto& clip = m_animationClips.at(m_currentAnim);
        //	float deltaTime = timeElapsed * clip.ticksPerSecond;
        //	float dt = std::min(deltaTime, 0.033f); // �ִ� 30FPS���� ���
        //	m_animTime += dt;
        //	if (m_animTime > clip.duration)
        //	{
        //		m_animTime = fmod(m_animTime, clip.duration);
        //		//m_animTime = 0.0f;
        //	}
        //	char buffer[128];
        //	sprintf_s(buffer, "AnimTime: %.4f\n", m_animTime);
        //	OutputDebugStringA(buffer);
        //}
    }

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
        if (m_animationClips.contains("Idle"))
            SetCurrentAnimation("Idle");
    }

    if (m_animationClips.contains(m_currentAnim))
    {
        const auto& clip = m_animationClips.at(m_currentAnim);
        m_animTime += timeElapsed * clip.ticksPerSecond;
        if (m_animTime > clip.duration)
            m_animTime = fmod(m_animTime, clip.duration);
    }

    {
        /////////////////////////����ٰ� GetPosition()����ؼ� float�� 3�� ������ ���� �ɰž�

    }

    XMFLOAT3 pos = GetPosition();
    m_boundingBox.Center = XMFLOAT3{
        pos.x,
        pos.y,  // �߽��� �Ǻ�(��)���� ���� ������ ����
        pos.z
    };

    GameObject::Update(timeElapsed);
}


void Player::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
    bool isShadowPass = m_shader && m_shader->IsShadowShader();

   //// �ִϸ��̼� �� ��� ���ε�
   //if (!isShadowPass && m_animationClips.contains(m_currentAnim))
   //{
   //    const auto& clip = m_animationClips.at(m_currentAnim);
   //    float animTime = fmod(m_animTime, clip.duration);
   //    auto [boneTransforms, animBoneIndex] = clip.GetBoneTransforms(animTime, m_boneNameToIndex, m_boneHierarchy,m_staticNodeTransforms);
   //    const_cast<Player*>(this)->UploadBoneMatricesToShader(boneTransforms, animBoneIndex, commandList);
   //}

    // �� ��� StructuredBuffer ���ε�
    if (!isShadowPass && m_boneMatrixSRV.ptr != 0)
    {
        commandList->SetGraphicsRootDescriptorTable(RootParameter::BoneMatrix, m_boneMatrixSRV);
    }

    // �� �޽� ������
    for (const auto& mesh : m_meshes)
    {
        if (m_shader) m_shader->UpdateShaderVariable(commandList); // ���̴� ����

        if (!isShadowPass)
        {
            if (m_texture)  m_texture->UpdateShaderVariable(commandList, m_textureIndex);
            if (m_material) m_material->UpdateShaderVariable(commandList);
        }

        // ��ӹ��� GameObject�� ������� (world matrix ��)
        UpdateShaderVariable(commandList);

        // �޽� ������
        mesh->Render(commandList);
    }

    //���̾� ������ ������
    if (!isShadowPass && m_drawBoundingBox && m_debugBoxMesh && m_debugLineShader)
    {
        m_debugLineShader->UpdateShaderVariable(commandList);
        m_debugBoxMesh->Render(commandList);
    }

}


void Player::UploadBoneMatricesToShader(const std::vector<XMMATRIX>& boneTransforms, const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    ////const UINT MAX_BONES = 128;
    ////vector<XMMATRIX> finalMatrices(MAX_BONES, XMMatrixIdentity());
    //
    ////size_t count = std::min<size_t>(boneTransforms.size(), MAX_BONES);
    //
    ////for (size_t i = 0; i < count; ++i)
    ////{
    ////    // �� offset�� �����Ѵٸ� ������
    ////    if (m_boneOffsets.contains(i))
    ////    {
    ////        finalMatrices[i] = XMMatrixMultiply(boneTransforms[i], m_boneOffsets.at(i));
    ////    }
    ////    else
    ////    {
    ////        finalMatrices[i] = boneTransforms[i]; // fallback
    ////    }
    ////}
    //const UINT MAX_BONES = boneTransforms.size();
    //vector<XMMATRIX> finalMatrices(MAX_BONES, XMMatrixIdentity());
    //
    //size_t count = std::min<size_t>(boneTransforms.size(), MAX_BONES);
    //
    //
    //for (const auto& [boneName, vertexIndex] : m_boneNameToIndex)
    //{
    //    // animBoneIndex: boneName �� i (�ִϸ��̼� transform�� �ε���)
    //    if (animBoneIndex.contains(boneName) && m_boneOffsets.contains(vertexIndex))
    //    {
    //        int animIndex = animBoneIndex.at(boneName);
    //        finalMatrices[vertexIndex] = XMMatrixMultiply(boneTransforms[animIndex], m_boneOffsets.at(vertexIndex));
    //    }
    //}
    //// GPU�� ���ε� (UploadBuffer �� DefaultBuffer)
    //D3D12_SUBRESOURCE_DATA subresourceData{};
    //subresourceData.pData = finalMatrices.data();
    //subresourceData.RowPitch = sizeof(XMMATRIX) * MAX_BONES;
    //subresourceData.SlicePitch = subresourceData.RowPitch;
    //
    //commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
    //    m_boneMatrixBuffer.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));
    //
    //UpdateSubresources<1>(commandList.Get(), m_boneMatrixBuffer.Get(), m_boneMatrixUploadBuffer.Get(), 0, 0, 1, &subresourceData);
    //
    //commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
    //    m_boneMatrixBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
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

void Player::PlayAnimationWithBlend(const std::string& newAnim, float blendDuration)
{
    if (m_currentAnim == newAnim || !m_animationClips.contains(newAnim)) return;

    m_nextAnim = newAnim;
    m_blendDuration = blendDuration;
    m_blendTime = 0.f;
    m_isBlending = true;
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
