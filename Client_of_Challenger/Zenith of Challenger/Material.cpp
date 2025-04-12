#include "material.h"

void Material::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	for (int i = 0; const auto & material : m_material) {
		m_constantBuffer->Copy(material, i++);
	}
	m_constantBuffer->UpdateRootConstantBuffer(commandList);
}

void Material::SetMaterial(MaterialData material)
{
	m_material.push_back(material);
}

void Material::SetMaterial(XMFLOAT3 fresnelR0, FLOAT roughness, XMFLOAT3 ambient)
{
	MaterialData material;
	material.fresnelR0 = fresnelR0;
	material.roughness = roughness;
	material.ambient = ambient;
	m_material.push_back(material);
}

void Material::SetAmbient(const XMFLOAT3& ambient)
{
	if (m_material.empty()) m_material.resize(1);
	m_material[0].ambient = ambient;
}

void Material::SetFresnelR0(const XMFLOAT3& fresnelR0)
{
	if (m_material.empty()) m_material.resize(1);
	m_material[0].fresnelR0 = fresnelR0;
}

void Material::SetRoughness(float roughness)
{
	if (m_material.empty()) m_material.resize(1);
	m_material[0].roughness = roughness;
}

void Material::CreateShaderVariable(const ComPtr<ID3D12Device>& device)
{
	if (m_material.empty()) {
		// 기본값으로 하나 추가
		MaterialData defaultMat;
		defaultMat.fresnelR0 = XMFLOAT3(0.04f, 0.04f, 0.04f);
		defaultMat.roughness = 0.8f;
		defaultMat.ambient = XMFLOAT3(0.05f, 0.05f, 0.05f);
		m_material.push_back(defaultMat);
	}

	m_constantBuffer = make_unique<UploadBuffer<MaterialData>>(device,
		static_cast<UINT>(RootParameter::Material), static_cast<UINT>(m_material.size()), true);
}