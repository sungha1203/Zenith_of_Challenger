//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------
#include "object.h"
#include "Instance.h"
#include "GameFramework.h"

Object::Object() :
	m_right{ 1.f, 0.f, 0.f }, m_up{ 0.f, 1.f, 0.f }, m_front{ 0.f, 0.f, 1.f }
{
	XMStoreFloat4x4(&m_worldMatrix, XMMatrixIdentity());
}

void Object::Transform(XMFLOAT3 shift)
{
	SetPosition(Vector3::Add(GetPosition(), shift));
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
	m_worldMatrix._41 = position.x;
	m_worldMatrix._42 = position.y;
	m_worldMatrix._43 = position.z;
}

XMFLOAT3 Object::GetPosition() const
{
	return XMFLOAT3{ m_worldMatrix._41, m_worldMatrix._42, m_worldMatrix._43 };
}

void Object::UpdateWorldMatrix()
{
	XMMATRIX scaleMatrix = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
	XMMATRIX translationMatrix = XMMatrixTranslation(m_worldMatrix._41, m_worldMatrix._42, m_worldMatrix._43);
	XMMATRIX worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	XMStoreFloat4x4(&m_worldMatrix, worldMatrix);
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
	if (!m_isVisible) return; //visibleÀÌ false¸é ·»´õ¸µ skip

	if (m_shader)
	{
		m_shader->UpdateShaderVariable(commandList); // ¼ÎÀÌ´õ ¼³Á¤
	}
	UpdateShaderVariable(commandList);

	if (m_texture) m_texture->UpdateShaderVariable(commandList, m_textureIndex);
	if (m_material) m_material->UpdateShaderVariable(commandList);

	m_mesh->Render(commandList);

	if (m_drawBoundingBox && m_debugBoxMesh && m_debugLineShader)
	{
		m_debugLineShader->UpdateShaderVariable(commandList); // View/Proj
		UpdateShaderVariable(commandList); // World matrix µî

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
	if (!m_textureIndex)
	{
		buffer.textureIndex = m_textureIndex;
	}
	else
	{
		buffer.textureIndex = m_textureIndex - 1;
	}
	buffer.isHovered = m_isHovered ? 1 : 0;
	buffer.padding = FLOAT(0.f);

	m_constantBuffer->Copy(buffer);
	m_constantBuffer->UpdateRootConstantBuffer(commandList);

	if (m_texture) m_texture->UpdateShaderVariable(commandList, m_textureIndex);
	if (m_material) m_material->UpdateShaderVariable(commandList);
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

	XMFLOAT3 e = box.Extents;

	XMFLOAT3 corners[8] = {
		{-e.x, -e.y, -e.z},
		{-e.x, +e.y, -e.z},
		{+e.x, +e.y, -e.z},
		{+e.x, -e.y, -e.z},
		{-e.x, -e.y, +e.z},
		{-e.x, +e.y, +e.z},
		{+e.x, +e.y, +e.z},
		{+e.x, -e.y, +e.z}
	};

	std::vector<UINT> indices = {
		// ¾Õ¸é (-z)
		0, 1, 2,
		0, 2, 3,

		// µÞ¸é (+z)
		4, 6, 5,
		4, 7, 6,

		// ¿ÞÂÊ¸é (-x)
		0, 4, 5,
		0, 5, 1,

		// ¿À¸¥ÂÊ¸é (+x)
		3, 2, 6,
		3, 6, 7,

		// À­¸é (+y)
		1, 5, 6,
		1, 6, 2,

		// ¾Æ·§¸é (-y)
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
