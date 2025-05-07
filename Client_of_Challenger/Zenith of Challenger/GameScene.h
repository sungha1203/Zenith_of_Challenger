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
    void SetInventoryCount(int item , int num) { m_inventoryCounts[item] = num; }
    void SetupgradeScore(int num) { m_upgradeScore = num; }

    //장비창 관련
    void HandleMouseClick(int mouseX, int mouseY);
    void SetWeaponSlotUV(int type);
    void SetJobSlotUV(int type);
    void UpdateEnhanceDigits();

private:
    shared_ptr<FBXLoader> m_fbxLoader; // FBX 로더 추가
    shared_ptr<FBXLoader> m_ZenithLoader; // FBX 로더 추가
    shared_ptr<FBXLoader> m_playerLoader;
    vector<shared_ptr<MeshBase>> m_fbxMeshes; // FBX에서 로드한 메쉬 저장
    vector<shared_ptr<MeshBase>> m_ZenithMeshes; // FBX에서 로드한 메쉬 저장
    vector<shared_ptr<GameObject>> m_fbxObjects; // FBX 모델용 GameObject 리스트 추가
    vector<shared_ptr<GameObject>> m_ZenithObjects; // 정점맵 오브젝트 저장


    bool m_debugDrawEnabled = false;
    bool m_ShadowMapEnabled = false;
    
    //정점맵 토글키
    bool m_ZenithEnabled = false;


    ////////////////몬스터 관련////////////////
    unordered_map<string, vector<shared_ptr<Monsters>>> m_monsterGroups;
    unordered_map<string, shared_ptr<MeshBase>> m_meshLibrary;


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
    
    //인벤토리 무기 및 전직서 관련
    bool m_WeaponOnly = false;
    bool m_JopOnly = false;

    //강화창 관련
    std::shared_ptr<GameObject> m_reinforcedWindowUI;
    bool m_showReinforcedWindow = false;

    bool m_isReinforceWindowVisible = false; // 'I'키 토글
    bool m_isReinforceSlotOccupied = false;

    std::string m_selectedItemType = ""; // "weapon", "job"

    std::shared_ptr<GameObject> m_weaponSlotIcon;
    std::shared_ptr<GameObject> m_jobSlotIcon;
    std::shared_ptr<GameObject> m_plusIcon;

    int m_upgradeScore = 0;
    std::vector<std::shared_ptr<GameObject>> m_forcedDigits;

    //툰 렌더링 외곽선 토글키
    bool m_OutLine = false;
    unordered_map<string, unordered_map<string, AnimationClip>> m_animClipLibrary;
    unordered_map<string, unordered_map<int, XMMATRIX>> m_boneOffsetLibrary;
    unordered_map<string, unordered_map<string, int>> m_boneNameMap;
    unordered_map<string, unordered_map<string, string>> m_BoneHierarchy;
    unordered_map<string, unordered_map<string, XMMATRIX>> m_staticNodeTransforms;
    unordered_map<string, unordered_map<string, XMMATRIX>> m_NodeNameToGlobalTransform;
};