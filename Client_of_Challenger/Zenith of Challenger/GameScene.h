#pragma once
#include "Scene.h"
#include "GameFramework.h"
#include "FBXLoader.h"
#include "Monsters.h"
#include "ParticleEffect.h"
#include "ParticleManager.h"

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

    void AddCubeCollider(const XMFLOAT3& position, const XMFLOAT3& extents, const FLOAT& rotate = 0.f);

    void RenderShadowPass(const ComPtr<ID3D12GraphicsCommandList>& commandList);

    unordered_map<string, vector<shared_ptr<Monsters>>>& GetMonsterGroups() { return m_monsterGroups; }
    shared_ptr<ParticleManager> GetParticleManager() const { return m_particleManager; }
    void SetGoldScore(int score) { m_goldScore = score; }

private:
    shared_ptr<FBXLoader> m_fbxLoader; // FBX 로더 추가
    shared_ptr<FBXLoader> m_playerLoader;
    vector<shared_ptr<MeshBase>> m_fbxMeshes; // FBX에서 로드한 메쉬 저장
    vector<shared_ptr<GameObject>> m_fbxObjects; // FBX 모델용 GameObject 리스트 추가


    bool m_debugDrawEnabled = false;
    bool m_ShadowMapEnabled = false;


    ////////////////몬스터 관련////////////////
    unordered_map<string, vector<shared_ptr<Monsters>>> m_monsterGroups;
    unordered_map<string, shared_ptr<MeshBase>> m_meshLibrary;
    unordered_map<string, AnimationClip> m_animClipLibrary;
    unordered_map<string, XMMATRIX> m_boneOffsetLibrary;
    unordered_map<string, int> m_boneNameMap;


    //그림자 관련
    std::shared_ptr<DebugShadowShader> m_debugShadowShader;
    XMMATRIX m_shadowViewMatrix;
    XMMATRIX m_shadowProjMatrix;


    //카메라 전환 관련
    CameraMode m_currentCameraMode = CameraMode::QuarterView;
    shared_ptr<QuarterViewCamera> m_quarterViewCamera;
    shared_ptr<ThirdPersonCamera> m_thirdPersonCamera;

    //골드 관련
    int m_goldScore = 0; // Gold 점수
    std::vector<std::shared_ptr<GameObject>> m_goldDigits; // 각 자릿수마다 UI 오브젝트를 저장

    //파티클 관련
    shared_ptr<ParticleManager> m_particleManager;

    //인벤토리 숫자 관련
    std::vector<std::shared_ptr<GameObject>> m_inventoryDigits;
    int m_inventoryCounts[6] = { 0, 0, 0, 0, 0, 0 };
};