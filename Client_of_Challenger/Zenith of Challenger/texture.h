#pragma once
#include "stdafx.h"

class Texture
{
public:
	Texture() = delete;
	Texture(const ComPtr<ID3D12Device>& device);
	Texture(const ComPtr<ID3D12Device>& device,
		const ComPtr<ID3D12GraphicsCommandList>& commandList,
		const wstring& fileName, UINT rootParameterIndex, BOOL createResourceView = true);
	~Texture() = default;

	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList, int textureIndexOverride) const;
	void CreateShaderVariable(const ComPtr<ID3D12Device>& device, bool useGlobalHeap = false);

	void ReleaseUploadBuffer();

	void LoadTexture(const ComPtr<ID3D12Device>& device,
		const ComPtr<ID3D12GraphicsCommandList>& commandList,
		const wstring& fileName, UINT rootParameterIndex);
	int GetTextureIndex() const { return m_textureIndex; }

private:
	void CreateSrvDescriptorHeap(const ComPtr<ID3D12Device>& device);
	void CreateShaderResourceView(const ComPtr<ID3D12Device>& device);


private:
	UINT m_srvDescriptorSize;

	ComPtr<ID3D12DescriptorHeap>				m_srvDescriptorHeap;
	UINT										m_rootParameterIndex;
	vector<ComPtr<ID3D12Resource>>				m_textures;
	vector<ComPtr<ID3D12Resource>>				m_textureUploadBuffer;

	D3D12_GPU_DESCRIPTOR_HANDLE m_srvHandle{}; // 셰이더 바인딩용 GPU 핸들 저장
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_gpuHandles;

	bool m_useGlobalHeap = false;
	int m_textureIndex = 0;
};