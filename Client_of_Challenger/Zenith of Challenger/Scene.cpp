//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------
#include "Scene.h"
#include "stdafx.h"
#include "GameFramework.h"



Scene::Scene()
{
}

void Scene::MouseEvent(HWND hWnd, FLOAT timeElapsed)
{
}

void Scene::KeyboardEvent(FLOAT timeElapsed)
{
}

void Scene::Update(FLOAT timeElapsed)
{
}

void Scene::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
}

void Scene::PreRender(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
}

void Scene::BuildObjects(const ComPtr<ID3D12Device>& device,
	const ComPtr<ID3D12GraphicsCommandList>& commandList,
	const ComPtr<ID3D12RootSignature>& rootSignature)
{
}

void Scene::BuildShaders(const ComPtr<ID3D12Device>& device,
	const ComPtr<ID3D12GraphicsCommandList>& commandList,
	const ComPtr<ID3D12RootSignature>& rootSignature)
{
}

void Scene::BuildMeshes(const ComPtr<ID3D12Device>& device,
	const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
}

void Scene::BuildTextures(const ComPtr<ID3D12Device>& device,
	const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
}

void Scene::BuildMaterials(const ComPtr<ID3D12Device>& device,
	const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
}


void Scene::BuildObjects(const ComPtr<ID3D12Device>& device)
{
}

void Scene::ReleaseUploadBuffer()
{
	for (auto& mesh : views::values(m_meshes)) {
		mesh->ReleaseUploadBuffer();
	}
	for (auto& texture : views::values(m_textures)) {
		texture->ReleaseUploadBuffer();
	}
}

void Scene::MouseEvent(UINT message, LPARAM lParam)
{
}

void Scene::KeyboardEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
}

void Scene::KeyboardEvent(UINT message, WPARAM wParam)
{
}

void Scene::ClearSceneResources()
{
	std::cout << "Scene 리소스 완전 해제 중..." << std::endl;

	//for (auto& [name, mesh] : m_meshes)
	//{
	//	char msg[128];
	//	sprintf_s(msg, "[Clear] Mesh being cleared: %p\n", mesh.get());
	//	OutputDebugStringA(msg);
	//}

	//모든 메쉬, 텍스처, 오브젝트 삭제
	m_meshes.clear();
	m_textures.clear();
	m_objects.clear();

	//GPU 리소스 해제
	if (m_device)
	{
		m_device->GetDeviceRemovedReason();
	}
}

void Scene::AddTrailObject(const std::shared_ptr<GameObject>& obj)
{
}
