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
        D3D12_RESOURCE_STATE_GENERIC_READ); // �� ���� ���� ����!

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

    // �ִϸ��̼� �� ��� ���ε�
    if (!isShadowPass && m_animationClips.contains(m_currentAnim))
    {
        const auto& clip = m_animationClips.at(m_currentAnim);
        float animTime = fmod(m_animTime, clip.duration);
        auto [boneTransforms, animBoneIndex] = clip.GetBoneTransforms(animTime, m_boneNameToIndex, m_boneHierarchy,m_staticNodeTransforms);
        const_cast<Player*>(this)->UploadBoneMatricesToShader(boneTransforms, animBoneIndex, commandList);
    }

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


void Player::UploadBoneMatricesToShader(const std::vector<XMMATRIX>& boneTransforms, std::unordered_map<std::string, int>animBoneIndex, const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    //const UINT MAX_BONES = 128;
    //vector<XMMATRIX> finalMatrices(MAX_BONES, XMMatrixIdentity());

    //size_t count = std::min<size_t>(boneTransforms.size(), MAX_BONES);

    //for (size_t i = 0; i < count; ++i)
    //{
    //    // �� offset�� �����Ѵٸ� ������
    //    if (m_boneOffsets.contains(i))
    //    {
    //        finalMatrices[i] = XMMatrixMultiply(boneTransforms[i], m_boneOffsets.at(i));
    //    }
    //    else
    //    {
    //        finalMatrices[i] = boneTransforms[i]; // fallback
    //    }
    //}
    const UINT MAX_BONES = boneTransforms.size();
    vector<XMMATRIX> finalMatrices(MAX_BONES, XMMatrixIdentity());

    size_t count = std::min<size_t>(boneTransforms.size(), MAX_BONES);


    for (const auto& [boneName, vertexIndex] : m_boneNameToIndex)
    {
        // animBoneIndex: boneName �� i (�ִϸ��̼� transform�� �ε���)
        if (animBoneIndex.contains(boneName) && m_boneOffsets.contains(vertexIndex))
        {
            int animIndex = animBoneIndex.at(boneName);
            finalMatrices[vertexIndex] = XMMatrixMultiply(boneTransforms[animIndex], m_boneOffsets.at(vertexIndex));
        }
    }
    // GPU�� ���ε� (UploadBuffer �� DefaultBuffer)
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
