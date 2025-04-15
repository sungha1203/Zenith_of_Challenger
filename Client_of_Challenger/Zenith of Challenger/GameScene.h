#pragma once
#include "Scene.h"
#include "GameFramework.h"
#include "FBXLoader.h"
#include "Monsters.h"

class FBXLoader; // 전방 선언 추가

class GameScene : public Scene
{
public:
    GameScene() = default;
    ~GameScene() override = default;

    void BuildObjects(const ComPtr<ID3D12Device>& device,
        const ComPtr<ID3D12GraphicsCommandList>& commandList,
        const ComPtr<ID3D12RootSignature>& rootSignature) override;

    virtual void MouseEvent(HWND hWnd, FLOAT timeElapsed);
    virtual void KeyboardEvent(FLOAT timeElapsed);

    virtual void Update(FLOAT timeElapsed);
    virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;
    virtual void PreRender(const ComPtr<ID3D12GraphicsCommandList>& commandList);

    virtual void BuildShaders(const ComPtr<ID3D12Device>& device,
        const ComPtr<ID3D12GraphicsCommandList>& commandList,
        const ComPtr<ID3D12RootSignature>& rootSignature);
    virtual void BuildMeshes(const ComPtr<ID3D12Device>& device,
        const ComPtr<ID3D12GraphicsCommandList>& commandList);
    virtual void BuildTextures(const ComPtr<ID3D12Device>& device,
        const ComPtr<ID3D12GraphicsCommandList>& commandList);
    virtual void BuildMaterials(const ComPtr<ID3D12Device>& device,
        const ComPtr<ID3D12GraphicsCommandList>& commandList);
    virtual void BuildObjects(const ComPtr<ID3D12Device>& device);

private:
    shared_ptr<FBXLoader> m_fbxLoader; // FBX 로더 추가
    shared_ptr<FBXLoader> m_playerLoader;
    vector<shared_ptr<MeshBase>> m_fbxMeshes; // FBX에서 로드한 메쉬 저장
    vector<shared_ptr<GameObject>> m_fbxObjects; // FBX 모델용 GameObject 리스트 추가

    vector<shared_ptr<Monsters>> m_Monsters; 

    bool m_debugDrawEnabled = false;

    //std::unique_ptr<BoundingBoxHelper> m_boxHelper;
};
