#pragma once
#include "Scene.h"
#include "GameFramework.h"
#include "FBXLoader.h"
#include "Monsters.h"

class FBXLoader; // ���� ���� �߰�

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

    void AddCubeCollider(const XMFLOAT3& position, const XMFLOAT3& extents, const FLOAT& rotate = 0.f);

    void RenderShadowPass(const ComPtr<ID3D12GraphicsCommandList>& commandList);

private:
    shared_ptr<FBXLoader> m_fbxLoader; // FBX �δ� �߰�
    shared_ptr<FBXLoader> m_playerLoader;
    vector<shared_ptr<MeshBase>> m_fbxMeshes; // FBX���� �ε��� �޽� ����
    vector<shared_ptr<GameObject>> m_fbxObjects; // FBX �𵨿� GameObject ����Ʈ �߰�


    bool m_debugDrawEnabled = false;
    bool m_ShadowMapEnabled = false;


    ////////////////���� ����////////////////
    unordered_map<string, vector<shared_ptr<Monsters>>> m_monsterGroups;
    unordered_map<string, shared_ptr<MeshBase>> m_meshLibrary;
    unordered_map<string, AnimationClip> m_animClipLibrary;
    unordered_map<string, XMMATRIX> m_boneOffsetLibrary;
    unordered_map<string, int> m_boneNameMap;


    //�׸��� ����
    std::shared_ptr<DebugShadowShader> m_debugShadowShader;
    XMMATRIX m_shadowViewMatrix;
    XMMATRIX m_shadowProjMatrix;
};