#pragma once
#include "stdafx.h"
#include "buffer.h"

//재질은 어떤 물체가 빛을 얼마나 반사하거나 흡수할지, 물체의 표면 거칠기는 어떠한지, 물체가 광원으로 부터 직접 빛을 받지
//않더라도 기본적으로 얼마나 빛을 받도록 계산할 지를 저장.

struct MaterialData : public BufferBase
{
	XMFLOAT3 fresnelR0; //해당 재질로 입사한 빛을 얼마나 반사하느냐 결정 (반사 정도)
	FLOAT roughness; //재질의 거칠기
	XMFLOAT3 ambient; //물체가 직접 빛을 받지 한고 주변 물체의 반사로 인해 들어오는 빛을 보정해 주는 값
};

class Material
{
public:
	Material() = default;

	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;

	void SetMaterial(MaterialData material);
	void SetMaterial(XMFLOAT3 fresnelR0, FLOAT roughness, XMFLOAT3 ambient);
	void SetAmbient(const XMFLOAT3& ambient);
	void SetFresnelR0(const XMFLOAT3& fresnelR0);
	void SetRoughness(float roughness);

	void CreateShaderVariable(const ComPtr<ID3D12Device>& device);

private:
	vector<MaterialData> m_material;
	unique_ptr<UploadBuffer<MaterialData>> m_constantBuffer;
};