//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------
#pragma once
#include "mesh.h"
#include "Shader.h"
#include "texture.h"
#include "Camera.h"
#include "Player.h"
#include "object.h"
#include "stdafx.h"
#include "Instance.h"
#include "Lighting.h"
#include "OtherPlayer.h"
class Scene
{
public:
    Scene();
    virtual ~Scene() = default;

    virtual void MouseEvent(HWND hWnd, FLOAT timeElapsed);
    virtual void KeyboardEvent(FLOAT timeElapsed);
    virtual void Update(FLOAT timeElapsed);
    virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;
    virtual void PreRender(const ComPtr<ID3D12GraphicsCommandList>& commandList);
    virtual void BuildObjects(const ComPtr<ID3D12Device>& device,
        const ComPtr<ID3D12GraphicsCommandList>& commandList,
        const ComPtr<ID3D12RootSignature>& rootSignature);
    virtual void ReleaseUploadBuffer();

    virtual void MouseEvent(UINT message, LPARAM lParam);
    virtual void KeyboardEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void KeyboardEvent(UINT message, WPARAM wParam);

    virtual void ClearSceneResources(); //리소스 완전 해제 함수 추가

    void SetDevice(const ComPtr<ID3D12Device>& device) { m_device = device; }
    void SetCommandList(const ComPtr<ID3D12GraphicsCommandList>& commandList) { m_commandList = commandList; }
    void SetRootSignature(const ComPtr<ID3D12RootSignature>& rootSignature) { m_rootSignature = rootSignature; }

    virtual void AddTrailObject(const std::shared_ptr<GameObject>& obj);

    int otherid[2] = { -2,-2 };
    XMFLOAT3 otherpos[2];
    float otherangle[2];
    shared_ptr<OtherPlayer> m_Otherplayer[2];
    shared_ptr<Player> m_player;
protected:
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

protected:
    unordered_map<string, shared_ptr<Shader>> m_shaders;
    unordered_map<string, shared_ptr<MeshBase>> m_meshes;
    unordered_map<string, shared_ptr<Texture>> m_textures;
    unordered_map<string, shared_ptr<Material>> m_materials;

    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    ComPtr<ID3D12RootSignature> m_rootSignature;

    unique_ptr<LightSystem> m_lightSystem;
    unique_ptr<Sun>      m_sun;

    shared_ptr<Camera> m_camera;


    vector<shared_ptr<GameObject>> m_objects;
    shared_ptr<GameObject> m_skybox;
    shared_ptr<Terrain> m_terrain;

    unique_ptr<Instance> m_instanceObject;

    vector<shared_ptr<GameObject>> m_StartSceneObjects; //StartScene 전용 멤버변수
    vector<shared_ptr<GameObject>> m_SelectSceneObjects; //StartScene Select전용 멤버변수
    vector<shared_ptr<GameObject>> m_startBar;

    vector<std::shared_ptr<GameObject>> m_idObjects;
    vector<std::shared_ptr<GameObject>> m_pwObjects;
    vector<shared_ptr<GameObject>> m_joinButtons; // 참가 버튼
    vector<shared_ptr<GameObject>> m_uiObjects; //GameScene UI 변수
};
