//-----------------------------------------------------------------------------
// File: Monsters.cpp
//-----------------------------------------------------------------------------
#include "Monsters.h"
#include "GameFramework.h"

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

    //ü�¹� ����
    m_healthBar = make_shared<HealthBarObject>(device);
}

void Monsters::MouseEvent(FLOAT timeElapsed)
{
    // �ʿ� �� �ۼ�
}

void Monsters::KeyboardEvent(FLOAT timeElapsed)
{
    // AI ���ʹ� ���� �Է¹��� ����
}

void Monsters::Update(FLOAT timeElapsed)
{
    // ���� AI�� Ÿ�� ���� ������ Ȯ�� ����

    // �ִϸ��̼� ����
    if (m_animationClips.contains(m_currentAnim))
    {
        const auto& clip = m_animationClips.at(m_currentAnim);
        m_animTime += timeElapsed * clip.ticksPerSecond;
        if (m_animTime > clip.duration)
            m_animTime = fmod(m_animTime, clip.duration);
    }


    // [1] �÷��̾� ��ġ �޾ƿ���
    if (!gGameFramework->GetPlayer()) return;

    XMFLOAT3 targetPos = gGameFramework->GetPlayer()->GetPosition();
    XMFLOAT3 myPos = GetPosition();

    // [2] ���� ���� ���
    XMFLOAT3 toPlayer = {
        targetPos.x - myPos.x,
        0.f, // Y�� ȸ���̹Ƿ� ���� ����
        targetPos.z - myPos.z
    };

    if (!Vector3::IsZero(toPlayer))
    {
        toPlayer = Vector3::Normalize(toPlayer);

        // [3] ȸ�� ���� ��� (Z ����)
        float angle = atan2f(toPlayer.x, toPlayer.z); // x/z
        float degrees = XMConvertToDegrees(angle);

        // [4] ���Ͱ� �÷��̾ ���ϵ��� Y�� ȸ��
        SetRotationY(degrees + 180.f);
    }


    GameObject::Update(timeElapsed);

    if (m_healthBar)
    {
        m_healthBar->SetPosition(XMFLOAT3(m_position.x, m_position.y + 11.0f, m_position.z));
        m_healthBar->SetHP(m_currentHP, m_maxHP);
        m_healthBar->Update(timeElapsed, m_camera);
    }

}

void Monsters::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
    bool isShadowPass = m_shader && m_shader->IsShadowShader();

    //if (m_animationClips.contains(m_currentAnim))
    //{
    //    const auto& clip = m_animationClips.at(m_currentAnim);
    //    float animTime = fmod(m_animTime, clip.duration);
    //    auto boneTransforms = clip.GetBoneTransforms(animTime);
    //    const_cast<Monsters*>(this)->UploadBoneMatricesToShader(boneTransforms, commandList);
    //}

    //if (m_boneMatrixSRV.ptr != 0)
    //{
    //    commandList->SetGraphicsRootDescriptorTable(RootParameter::BoneMatrix, m_boneMatrixSRV);
    //}

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
    }

    //���̾� ������ ������
    if (!isShadowPass && m_drawBoundingBox && m_debugBoxMesh && m_debugLineShader)
    {
        m_debugLineShader->UpdateShaderVariable(commandList);
        m_debugBoxMesh->Render(commandList);

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

void Monsters::UploadBoneMatricesToShader(const unordered_map<string, XMMATRIX>& boneTransforms, const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    const UINT MAX_BONES = 128;
    vector<XMMATRIX> finalMatrices(MAX_BONES, XMMatrixIdentity());

    for (const auto& [name, transform] : boneTransforms)
    {
        if (m_boneNameToIndex.contains(name) && m_boneOffsets.contains(name))
        {
            int index = m_boneNameToIndex[name];
            if (index < MAX_BONES)
            {
                finalMatrices[index] = XMMatrixMultiply(transform, m_boneOffsets[name]);
            }
        }
    }

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

void Monsters::CreateBoneMatrixSRV(const ComPtr<ID3D12Device>& device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = 128;
    srvDesc.Buffer.StructureByteStride = sizeof(XMMATRIX);
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

    device->CreateShaderResourceView(m_boneMatrixBuffer.Get(), &srvDesc, cpuHandle);
    m_boneMatrixSRV = gpuHandle;
}

void Monsters::SetHP(int hp)
{
    m_currentHP = hp;
    if (m_healthBar) m_healthBar->SetHP(m_currentHP, m_maxHP);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HealthBarObject::HealthBarObject(const ComPtr<ID3D12Device>& device) : GameObject(device)
{
    // ü�¹ٿ� ���� �簢�� ���� (Position + Normal + UV)
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

    m_baseColor = XMFLOAT4(1.f, 0.f, 0.f, 1.f); // �⺻ ������
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
    // ü�� ���� ���
    float ratio = m_currentHP / m_maxHP;
    ratio = max(0.0f, min(1.0f, ratio)); // 0~1 ������ Ŭ����

    // ü�� ������ ���� ������ ���� (���θ�)
    SetScale(XMFLOAT3(ratio, 1.f, 1.f));

    // ü�·��� ���� ���� �ٲ� �� ���� (����)
    if (ratio > 0.5f)
        m_baseColor = XMFLOAT4(1.f, 0.f, 0.f, 1.f); // �ʷϻ�
    else if (ratio > 0.2f)
        m_baseColor = XMFLOAT4(0.f, 1.f, 0.f, 1.f); // �����
    else
        m_baseColor = XMFLOAT4(0.f, 0.f, 1.f, 1.f); // ������

    if (camera)
    {
        XMFLOAT3 pos = GetPosition();

        // ī�޶� Basis ��������
        XMFLOAT3 cameraRight = camera->GetU();  // ������ ���� (X)
        XMFLOAT3 cameraUp = camera->GetV();     // ���� ���� (Y)

        // Billboard�� WorldMatrix ����
        XMMATRIX world =
            XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z) *
            XMMATRIX(
                XMLoadFloat3(&cameraRight),
                XMLoadFloat3(&cameraUp),
                XMVectorSet(0.f, 0.f, 1.f, 0.f), // ���̹��� ����
                XMVectorSet(pos.x, pos.y, pos.z, 1.f)
            );

        XMStoreFloat4x4(&m_worldMatrix, world);
    }

}
