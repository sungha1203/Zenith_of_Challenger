#include "texture.h"
#include "DDSTextureLoader12.h"
#include "WICTextureLoader12.h" // 새로 추가 (WIC 사용 위해)
#include "GameFramework.h"

Texture::Texture(const ComPtr<ID3D12Device>& device)
{
	m_srvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

Texture::Texture(const ComPtr<ID3D12Device>& device,
	const ComPtr<ID3D12GraphicsCommandList>& commandList,
	const wstring& fileName, UINT rootParameterIndex, BOOL createResourceView)
{
	m_srvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	LoadTexture(device, commandList, fileName, rootParameterIndex);
	if (createResourceView) CreateShaderVariable(device);
}

void Texture::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList, int textureIndexOverride) const
{
	if (m_useGlobalHeap)
	{
		// 전역 디스크립터 힙 사용 시: Texture 배열이므로 첫 번째 핸들만 바인딩
		if (!m_gpuHandles.empty())
		{
			auto baseHandle = gGameFramework->GetGPUHeapStart();
			UINT descriptorSize = gGameFramework->GetDescriptorSize();

			D3D12_GPU_DESCRIPTOR_HANDLE offsetHandle = baseHandle;
			offsetHandle.ptr += static_cast<UINT64>(textureIndexOverride) * descriptorSize;

			commandList->SetGraphicsRootDescriptorTable(m_rootParameterIndex, offsetHandle);
		}
	}
	else
	{
		ID3D12DescriptorHeap* ppHeaps[] = { m_srvDescriptorHeap.Get() };
		commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		commandList->SetGraphicsRootDescriptorTable(m_rootParameterIndex, m_srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	}
}

void Texture::ReleaseUploadBuffer()
{
	for (auto& uploadBuffer : m_textureUploadBuffer) {
		uploadBuffer.Reset();
	}
	m_textureUploadBuffer.clear();
}

void Texture::LoadTexture(const ComPtr<ID3D12Device>& device,
	const ComPtr<ID3D12GraphicsCommandList>& commandList,
	const wstring& fileName, UINT rootParameterIndex)
{
	m_rootParameterIndex = rootParameterIndex;

	ComPtr<ID3D12Resource> texture;
	ComPtr<ID3D12Resource> textureUploadBuffer;

	unique_ptr<uint8_t[]> ddsData;
	vector<D3D12_SUBRESOURCE_DATA> subresources;
	DDS_ALPHA_MODE ddsAlphaMode{ DDS_ALPHA_MODE_UNKNOWN };
	DirectX::LoadDDSTextureFromFileEx(device.Get(), fileName.c_str(), 0,
		D3D12_RESOURCE_FLAG_NONE, DDS_LOADER_DEFAULT, texture.GetAddressOf(), ddsData, subresources, &ddsAlphaMode);

	UINT nSubresources{ (UINT)subresources.size() };
	const UINT64 TextureSize{ GetRequiredIntermediateSize(texture.Get(), 0, nSubresources) };

	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(TextureSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&textureUploadBuffer)
	);

	UpdateSubresources(commandList.Get(), texture.Get(), textureUploadBuffer.Get(), 0, 0, nSubresources, subresources.data());

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	m_textures.push_back(texture);
	m_textureUploadBuffer.push_back(textureUploadBuffer);
}

void Texture::CreateShaderVariable(const ComPtr<ID3D12Device>& device, bool useGlobalHeap)
{
	if (m_textures.empty()) return;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	if (m_rootParameterIndex == RootParameter::TextureCube) {
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE; // CubeMap
		srvDesc.TextureCube.MipLevels = 1;
	}
	else {
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 일반 텍스처
		srvDesc.Texture2D.MipLevels = 1;
	}

	m_useGlobalHeap = useGlobalHeap;
	m_gpuHandles.clear();

	if (useGlobalHeap)
	{
		UINT baseIndex = gGameFramework->GetCurrentSRVOffset(); // 시작 인덱스 저장

		for (const auto& tex : m_textures)
		{
			srvDesc.Format = tex->GetDesc().Format;

			auto [cpuHandle, gpuHandle] = gGameFramework->AllocateDescriptorHeapSlot();
			device->CreateShaderResourceView(tex.Get(), &srvDesc, cpuHandle);
			m_gpuHandles.push_back(gpuHandle);

			// 하나의 Texture 객체 기준, 첫 번째 텍스처의 index만 저장
			m_textureIndex = baseIndex;

#ifdef _DEBUG
			for (size_t i = 0; i < m_gpuHandles.size(); ++i)
			{
				char dbg[256];
				sprintf_s(dbg, "[CreateShaderVariable] RootParamIdx=%d, GPUHandle[%llu]=%llu\n",
					m_rootParameterIndex, i, m_gpuHandles[i].ptr);
				OutputDebugStringA(dbg);
			}

			char dbg2[128];
			sprintf_s(dbg2, "[Check] m_textureIndex = %d, m_gpuHandles.size() = %llu\n",
				m_textureIndex, m_gpuHandles.size());
			OutputDebugStringA(dbg2);
#endif
		}
	}
	else
	{
		CreateSrvDescriptorHeap(device);
		CreateShaderResourceView(device);
	}
}


void Texture::CreateSrvDescriptorHeap(const ComPtr<ID3D12Device>& device)
{
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = static_cast<UINT>(m_textures.size());
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	device->CreateDescriptorHeap(
		&srvHeapDesc, IID_PPV_ARGS(&m_srvDescriptorHeap));
}

void Texture::CreateShaderResourceView(const ComPtr<ID3D12Device>& device)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle{
		m_srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart() };

	for (const auto& texture : m_textures) {
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = texture->GetDesc().Format;

		switch (m_rootParameterIndex)
		{
		case RootParameter::Texture:
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = texture->GetDesc().MipLevels;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			break;
		case RootParameter::TextureCube:
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MostDetailedMip = 0;
			srvDesc.TextureCube.MipLevels = texture->GetDesc().MipLevels;
			srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
			break;
		default:
			break;
		}
		device->CreateShaderResourceView(texture.Get(), &srvDesc, descriptorHandle);

		descriptorHandle.Offset(1, m_srvDescriptorSize);
	}
}