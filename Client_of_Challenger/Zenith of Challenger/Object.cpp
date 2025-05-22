//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------
#include "object.h"
#include "Instance.h"
#include "GameFramework.h"

XMFLOAT4X4 XMMatrixToFloat4x4(const XMMATRIX& mat)
{
    XMFLOAT4X4 result;
    XMStoreFloat4x4(&result, mat);
    return result;
}

Object::Object() :
    m_right{ 1.f, 0.f, 0.f }, m_up{ 0.f, 1.f, 0.f }, m_front{ 0.f, 0.f, 1.f },
    m_scale{ 1.f, 1.f, 1.f }, m_rotation{ 0.f, 0.f, 0.f }, m_position{ 0.f, 0.f, 0.f }
{
    XMStoreFloat4x4(&m_worldMatrix, XMMatrixIdentity());
}

void Object::Transform(XMFLOAT3 shift)
{
    m_prevPosition = m_position; // 이동 전 위치 저장
    m_position = Vector3::Add(m_position, shift);

    CS_Packet_UPDATECOORD pkt;
    pkt.type = CS_PACKET_UPDATECOORD;
    pkt.x = m_position.x;
    pkt.y = m_position.y;
    pkt.z = m_position.z;
    pkt.angle = m_rotation.y;

    pkt.size = sizeof(pkt);
    gGameFramework->GetClientNetwork()->SendPacket(reinterpret_cast<const char*>(&pkt), pkt.size);

    UpdateWorldMatrix();
}

void Object::Rotate(FLOAT pitch, FLOAT yaw, FLOAT roll)
{
    XMMATRIX rotate{ XMMatrixRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll)) };
    XMStoreFloat4x4(&m_worldMatrix, rotate * XMLoadFloat4x4(&m_worldMatrix));

    XMStoreFloat3(&m_right, XMVector3TransformNormal(XMLoadFloat3(&m_right), rotate));
    XMStoreFloat3(&m_up, XMVector3TransformNormal(XMLoadFloat3(&m_up), rotate));
    XMStoreFloat3(&m_front, XMVector3TransformNormal(XMLoadFloat3(&m_front), rotate));
}

void Object::SetPosition(XMFLOAT3 position)
{
    //m_worldMatrix._41 = position.x;
    //m_worldMatrix._42 = position.y;
    //m_worldMatrix._43 = position.z;
    m_position = position;
    UpdateWorldMatrix();
}

XMFLOAT3 Object::GetPosition() const
{
    //return XMFLOAT3{ m_worldMatrix._41, m_worldMatrix._42, m_worldMatrix._43 };
    return m_position;
}

void Object::UpdateWorldMatrix()
{
    XMMATRIX scale = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
    XMMATRIX rot = XMMatrixRotationRollPitchYaw(
        XMConvertToRadians(m_rotation.x),
        XMConvertToRadians(m_rotation.y),
        XMConvertToRadians(m_rotation.z));
    XMMATRIX trans = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);

    XMStoreFloat4x4(&m_worldMatrix, scale * rot * trans);
}

void Object::SetRotationY(float yaw)
{
    m_rotation.y = yaw;
    UpdateWorldMatrix(); // 회전 반영해서 월드행렬 다시 계산
}
void Object::SetRotationZ(float pitch)
{
    m_rotation.z = pitch;
    UpdateWorldMatrix(); // 회전 반영해서 월드행렬 다시 계산
}
void Object::SetRotationX(float Roll)
{
    m_rotation.x = Roll;
    UpdateWorldMatrix(); // 회전 반영해서 월드행렬 다시 계산
}
void Object::PlusRotationY(float yaw)
{
    m_rotation.y += yaw;
    UpdateWorldMatrix(); // 회전 반영해서 월드행렬 다시 계산
}
void Object::PlusRotationZ(float pitch)
{
    m_rotation.z += pitch;
    UpdateWorldMatrix(); // 회전 반영해서 월드행렬 다시 계산
}
void Object::RotationX90()
{
    // 1. 기존 행렬을 XMMATRIX로 로드
    XMMATRIX currentWorld = XMLoadFloat4x4(&m_worldMatrix);

    // 2. 추가 회전 행렬 생성
    XMMATRIX rotX = XMMatrixRotationX(XMConvertToRadians(90.0f));

    // 3. 기존 행렬에 회전 행렬 곱하기 (회전 → 기존)
    // 순서 중요: 회전을 "추가"하려면 → rotY * rotZ * 기존
    XMMATRIX updated = XMMatrixMultiply(rotX,currentWorld);

    // 4. 다시 저장
    XMStoreFloat4x4(&m_worldMatrix, updated);
}

void Object::RotationY90()
{
    // 1. 기존 행렬을 XMMATRIX로 로드
    XMMATRIX currentWorld = XMLoadFloat4x4(&m_worldMatrix);

    // 2. 추가 회전 행렬 생성
    XMMATRIX rotY = XMMatrixRotationY(XMConvertToRadians(90.0f));

    // 3. 기존 행렬에 회전 행렬 곱하기 (회전 → 기존)
    // 순서 중요: 회전을 "추가"하려면 → rotY * rotZ * 기존
    XMMATRIX updated = XMMatrixMultiply(rotY, currentWorld);

    // 4. 다시 저장
    XMStoreFloat4x4(&m_worldMatrix, updated);
}

void Object::RotationZ90()
{
    // 1. 기존 행렬을 XMMATRIX로 로드
    XMMATRIX currentWorld = XMLoadFloat4x4(&m_worldMatrix);

    // 2. 추가 회전 행렬 생성
    XMMATRIX rotZ = XMMatrixRotationZ(XMConvertToRadians(90.0f));

    // 3. 기존 행렬에 회전 행렬 곱하기 (회전 → 기존)
    // 순서 중요: 회전을 "추가"하려면 → rotY * rotZ * 기존
    XMMATRIX updated = XMMatrixMultiply(rotZ, currentWorld);

    // 4. 다시 저장
    XMStoreFloat4x4(&m_worldMatrix, updated);
}


void Object::SetScale(XMFLOAT3 scale)
{
    m_scale = scale;
    UpdateWorldMatrix();
}

XMFLOAT3 Object::GetScale() const
{
    return m_scale;
}


InstanceObject::InstanceObject() : Object(),
m_textureIndex{ 0 }, m_materialIndex{ 0 }
{
}

void InstanceObject::Update(FLOAT timeElapsed)
{
}

void InstanceObject::UpdateShaderVariable(InstanceData& buffer)
{
    XMStoreFloat4x4(&buffer.worldMatrix,
        XMMatrixTranspose(XMLoadFloat4x4(&m_worldMatrix)));
    buffer.textureIndex = m_textureIndex;
    buffer.materialIndex = m_materialIndex;
}

void InstanceObject::SetTextureIndex(UINT textureIndex)
{
    m_textureIndex = textureIndex;
}

void InstanceObject::SetMaterialIndex(UINT materialIndex)
{
    m_materialIndex = materialIndex;
}


GameObject::GameObject(const ComPtr<ID3D12Device>& device) : Object()
{
    m_constantBuffer = make_unique<UploadBuffer<ObjectData>>(device, (UINT)RootParameter::GameObject, true);
}

void GameObject::Update(FLOAT timeElapsed)
{
}

void GameObject::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
    if (!m_isVisible) return; //visible이 false면 렌더링 skip

    bool isShadow = m_shader && m_shader->IsShadowShader();

    if (m_shader)
    {
        m_shader->UpdateShaderVariable(commandList); // 셰이더 설정
    }

    UpdateShaderVariable(commandList);

    if (!isShadow) {
        if (m_texture) m_texture->UpdateShaderVariable(commandList, m_textureIndex);
        if (m_material) m_material->UpdateShaderVariable(commandList);
    }

    m_mesh->Render(commandList);

    if (m_drawBoundingBox && m_debugBoxMesh && m_debugLineShader)
    {
        m_debugLineShader->UpdateShaderVariable(commandList);
        UpdateShaderVariable(commandList);
        m_debugBoxMesh->Render(commandList);
    }
}

void GameObject::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList, const XMFLOAT4X4* overrideMatrix) const
{
    ObjectData buffer;

    XMMATRIX matrix = overrideMatrix ? XMLoadFloat4x4(overrideMatrix) : XMLoadFloat4x4(&m_worldMatrix);
    XMStoreFloat4x4(&buffer.worldMatrix, XMMatrixTranspose(matrix));
    buffer.baseColor = m_baseColor;
    buffer.useTexture = m_useTexture;
    buffer.textureIndex = m_textureIndex;
    buffer.isHovered = m_isHovered ? 1 : 0;
    buffer.fillAmount = m_fillAmount;
    buffer.customUV = m_customUV;
    buffer.useCustomUV = m_useCustomUV;

    m_constantBuffer->Copy(buffer);
    m_constantBuffer->UpdateRootConstantBuffer(commandList);

    bool isShadow = m_shader && m_shader->IsShadowShader();

    if (!isShadow)
    {
        if (m_texture) m_texture->UpdateShaderVariable(commandList, m_textureIndex);
        if (m_material) m_material->UpdateShaderVariable(commandList);
    }
}

void GameObject::SetMesh(const shared_ptr<MeshBase>& mesh)
{
    m_mesh = mesh;
}

void GameObject::SetTexture(const shared_ptr<Texture>& texture)
{
    m_texture = texture;
    m_useTexture = TRUE;
}

void GameObject::SetMaterial(const shared_ptr<Material>& material)
{
    m_material = material;
}

void GameObject::SetShader(const shared_ptr<Shader>& shader)
{
    m_shader = shader;
}

void GameObject::SetBaseColor(const XMFLOAT4& color)
{
    m_baseColor = color;
}

void GameObject::SetUseTexture(bool use)
{
    m_useTexture = use;
}

void GameObject::SetHP(float currentHP, float maxHP)
{
}

void GameObject::SetWorldMatrix(const XMMATRIX& worldMatrix)
{
    XMStoreFloat4x4(&m_worldMatrix, worldMatrix);
}

void GameObject::SetSRV(D3D12_GPU_DESCRIPTOR_HANDLE srvHandle)
{
    m_srvHandle = srvHandle;
}

void GameObject::SetBoundingBox(const BoundingBox& box)
{
    m_boundingBox = box;

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

    m_debugBoxMesh = make_shared<Mesh<DebugVertex>>(
        gGameFramework->GetDevice(), gGameFramework->GetCommandList(),
        lines, D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void GameObject::SetPlayerBoundingBox(const BoundingBox& box)
{
    m_boundingBox = box;

    XMFLOAT3 c = box.Center;
    XMFLOAT3 e = box.Extents;

    // 현재 스케일이 0.0005 이므로 그에 맞게 와이어프레임 메쉬는 스케일 역보정
    float inverseScale = 1.0f / 0.0005f;

    XMFLOAT3 scaledExtents = {
        e.x * inverseScale,
        e.y * inverseScale,
        e.z * inverseScale
    };

    XMFLOAT3 corners[8] = {
        {c.x - scaledExtents.x, c.y - scaledExtents.y, c.z - scaledExtents.z},
        {c.x - scaledExtents.x, c.y + scaledExtents.y, c.z - scaledExtents.z},
        {c.x + scaledExtents.x, c.y + scaledExtents.y, c.z - scaledExtents.z},
        {c.x + scaledExtents.x, c.y - scaledExtents.y, c.z - scaledExtents.z},
        {c.x - scaledExtents.x, c.y - scaledExtents.y, c.z + scaledExtents.z},
        {c.x - scaledExtents.x, c.y + scaledExtents.y, c.z + scaledExtents.z},
        {c.x + scaledExtents.x, c.y + scaledExtents.y, c.z + scaledExtents.z},
        {c.x + scaledExtents.x, c.y - scaledExtents.y, c.z + scaledExtents.z}
    };

    std::vector<UINT> indices = {
        // 앞면 (-z)
        0, 1, 2, 0, 2, 3,
        // 뒷면 (+z)
        4, 6, 5, 4, 7, 6,
        // 왼쪽면 (-x)
        0, 4, 5, 0, 5, 1,
        // 오른쪽면 (+x)
        3, 2, 6, 3, 6, 7,
        // 윗면 (+y)
        1, 5, 6, 1, 6, 2,
        // 아랫면 (-y)
        0, 3, 7, 0, 7, 4
    };

    vector<DebugVertex> lines;
    for (int i = 0; i < indices.size(); ++i)
        lines.emplace_back(corners[indices[i]]);

    m_debugBoxMesh = make_shared<Mesh<DebugVertex>>(
        gGameFramework->GetDevice(), gGameFramework->GetCommandList(),
        lines, D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void GameObject::SetMonstersBoundingBox(const BoundingBox& box)
{
    m_boundingBox = box;

    XMFLOAT3 c = box.Center;
    XMFLOAT3 e = box.Extents;

    // 현재 스케일이 0.1로 적용되어 있다면 inverse 적용
    float inverseScale = 1.0f / 0.1f;

    XMFLOAT3 scaledExtents = {
        e.x * inverseScale,
        e.y * inverseScale,
        e.z * inverseScale
    };

    // 회전 행렬 구성 (Y 90도  Z 90도)
    XMMATRIX rotY = XMMatrixRotationY(XMConvertToRadians(90.f));
    XMMATRIX rotZ = XMMatrixRotationZ(XMConvertToRadians(90.f));
    XMMATRIX rotation = rotY * rotZ;

    // 코너 생성  회전 적용
    XMFLOAT3 corners[8] = {
        {c.x - scaledExtents.x, c.y - scaledExtents.y, c.z - scaledExtents.z},
        {c.x - scaledExtents.x, c.y + scaledExtents.y, c.z - scaledExtents.z},
        {c.x + scaledExtents.x, c.y + scaledExtents.y, c.z - scaledExtents.z},
        {c.x + scaledExtents.x, c.y - scaledExtents.y, c.z - scaledExtents.z},
        {c.x - scaledExtents.x, c.y - scaledExtents.y, c.z + scaledExtents.z},
        {c.x - scaledExtents.x, c.y + scaledExtents.y, c.z + scaledExtents.z},
        {c.x + scaledExtents.x, c.y + scaledExtents.y, c.z + scaledExtents.z},
        {c.x + scaledExtents.x, c.y - scaledExtents.y, c.z + scaledExtents.z}
    };

    // 회전 적용
    for (int i = 0; i < 8; ++i)
    {
        XMVECTOR p = XMLoadFloat3(&corners[i]);
        p = XMVector3TransformCoord(p, rotation);
        XMStoreFloat3(&corners[i], p);
    }

    std::vector<UINT> indices = {
        // 앞면 (-z)
        0, 1, 2, 0, 2, 3,
        // 뒷면 (+z)
        4, 6, 5, 4, 7, 6,
        // 왼쪽면 (-x)
        0, 4, 5, 0, 5, 1,
        // 오른쪽면 (+x)
        3, 2, 6, 3, 6, 7,
        // 윗면 (+y)
        1, 5, 6, 1, 6, 2,
        // 아랫면 (-y)
        0, 3, 7, 0, 7, 4
    };

    std::vector<DebugVertex> lines;
    for (int i = 0; i < indices.size(); ++i)
        lines.emplace_back(corners[indices[i]]);

    m_debugBoxMesh = make_shared<Mesh<DebugVertex>>(
        gGameFramework->GetDevice(), gGameFramework->GetCommandList(),
        lines, D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


void GameObject::SetFlyFrightBoundingBox(const BoundingBox& box)
{
    m_boundingBox = box;

    XMFLOAT3 c = box.Center;
    XMFLOAT3 e = box.Extents;

    // 현재 스케일이 0.0005 이므로 그에 맞게 와이어프레임 메쉬는 스케일 역보정
    float inverseScale = 1.0f / 0.05f;

    XMFLOAT3 scaledExtents = {
        e.x * inverseScale,
        e.y * inverseScale,
        e.z * inverseScale
    };

    XMFLOAT3 corners[8] = {
        {c.x - scaledExtents.x, c.y - scaledExtents.y, c.z - scaledExtents.z},
        {c.x - scaledExtents.x, c.y + scaledExtents.y, c.z - scaledExtents.z},
        {c.x + scaledExtents.x, c.y + scaledExtents.y, c.z - scaledExtents.z},
        {c.x + scaledExtents.x, c.y - scaledExtents.y, c.z - scaledExtents.z},
        {c.x - scaledExtents.x, c.y - scaledExtents.y, c.z + scaledExtents.z},
        {c.x - scaledExtents.x, c.y + scaledExtents.y, c.z + scaledExtents.z},
        {c.x + scaledExtents.x, c.y + scaledExtents.y, c.z + scaledExtents.z},
        {c.x + scaledExtents.x, c.y - scaledExtents.y, c.z + scaledExtents.z}
    };

    std::vector<UINT> indices = {
        // 앞면 (-z)
        0, 1, 2, 0, 2, 3,
        // 뒷면 (+z)
        4, 6, 5, 4, 7, 6,
        // 왼쪽면 (-x)
        0, 4, 5, 0, 5, 1,
        // 오른쪽면 (+x)
        3, 2, 6, 3, 6, 7,
        // 윗면 (+y)
        1, 5, 6, 1, 6, 2,
        // 아랫면 (-y)
        0, 3, 7, 0, 7, 4
    };

    vector<DebugVertex> lines;
    for (int i = 0; i < indices.size(); ++i)
        lines.emplace_back(corners[indices[i]]);

    m_debugBoxMesh = make_shared<Mesh<DebugVertex>>(
        gGameFramework->GetDevice(), gGameFramework->GetCommandList(),
        lines, D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void GameObject::SetCustomUV(float u0, float v0, float u1, float v1)
{
    m_customUV = { u0, v0, u1, v1 };
}

void GameObject::SetuseCustomUV(int useUV)
{
    m_useCustomUV = useUV;
}

bool GameObject::IsPointInside(int mouseX, int mouseY) const
{
    int w, h;
    RECT rect;
    GetClientRect(gGameFramework->GetHWND(), &rect);
    w = rect.right - rect.left;
    h = rect.bottom - rect.top;

    auto toPixelX = [&](float ndcX) {
        return static_cast<int>((ndcX + 1.0f) * 0.5f * w);
        };
    auto toPixelY = [&](float ndcY) {
        return static_cast<int>((1.0f - ndcY) * 0.5f * h); // Y축 반전 주의
        };

    XMFLOAT3 pos = GetPosition();
    XMFLOAT3 scale = GetScale();

    float leftNDC = pos.x - scale.x * 0.5f;
    float rightNDC = pos.x + scale.x * 0.5f;
    float topNDC = pos.y + scale.y * 0.5f;
    float bottomNDC = pos.y - scale.y * 0.5f;

    int margin = 1; // 마진 1픽셀 (정밀하게 맞추고 싶으면 0으로)

    int left = toPixelX(leftNDC) + margin;
    int right = toPixelX(rightNDC) - margin;
    int top = toPixelY(topNDC) + margin;
    int bottom = toPixelY(bottomNDC) - margin;

    return (mouseX >= left && mouseX <= right && mouseY >= top && mouseY <= bottom);
}

//외곽선 전용
void GameObject::RenderOutline(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
}

void GameObject::RenderBoundingBoxWithoutScale(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
    if (!m_debugBoxMesh || !m_debugLineShader) return;

    // 스케일이 제거된 월드행렬 생성
    XMMATRIX scaleMat = XMMatrixScaling(1.f, 1.f, 1.f); // 스케일 제거
    XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
    XMMATRIX transMat = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
    XMMATRIX worldMatrix = scaleMat * rotMat * transMat;

    // 셰이더 설정
    m_debugLineShader->UpdateShaderVariable(commandList);

    // 이 월드행렬만 넘기기
    UpdateShaderVariable(commandList, &XMMatrixToFloat4x4(worldMatrix));

    m_debugBoxMesh->Render(commandList);
}


RotatingObject::RotatingObject() : InstanceObject()
{
    std::random_device rd;
    std::mt19937 randomEngine(rd());
    std::uniform_real_distribution<float> dis(10.0f, 50.0f);
    m_rotatingSpeed = dis(randomEngine);
}

void RotatingObject::Update(FLOAT timeElapsed)
{
    Rotate(0.f, m_rotatingSpeed * timeElapsed, 0.f);
}


LightObject::LightObject(const shared_ptr<SpotLight>& light) :
    RotatingObject(), m_light{ light }
{
}

void LightObject::Update(FLOAT timeElapsed)
{
    m_light->SetPosition(GetPosition());
    m_light->SetDirection(Vector3::Normalize(m_front));
}

Sun::Sun(const shared_ptr<DirectionalLight>& light) : m_light{ light },
m_strength{ 1.f, 1.f, 1.f }, m_phi{ 0.f }, m_theta{ 0.f }, m_radius{ Settings::SunRadius }
{
}

void Sun::SetStrength(XMFLOAT3 strength)
{
    m_strength = strength;
}

void Sun::Update(FLOAT timeElapsed)
{
    XMFLOAT3 fixedPosition{
       0.0f,
       m_radius,
       0.0f
    };

    SetPosition(fixedPosition);

    XMFLOAT3 fixedDirection{
       0.0f,
       -1.0f,
       0.0f
    };

    m_light->SetDirection(fixedDirection);
    m_light->SetStrength(m_strength);
}

Terrain::Terrain(const ComPtr<ID3D12Device>& device) :
    GameObject(device)
{
}

FLOAT Terrain::GetHeight(FLOAT x, FLOAT z)
{
    const XMFLOAT3 position = GetPosition();
    return static_pointer_cast<TerrainMesh>(m_mesh)->
        GetHeight(x - position.x, z - position.z) + position.y + 0.3f;
}
