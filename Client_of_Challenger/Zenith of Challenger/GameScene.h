#pragma once
#include "Scene.h"
#include "GameFramework.h"
#include "FBXLoader.h"
#include "Monsters.h"
#include "ParticleEffect.h"
#include "ParticleManager.h"
#include "HealingObject.h"
#include "MagicBall.h"
#include "HealingEffectObject.h"
#include"Sword.h"
#include "DissolveDustEffectObject.h"
#include "AttackRangeIndicator.h"
#include "SwordAuraObject.h"
#include"Staff.h"
#include"Shield.h"

class FBXLoader; // ���� ���� �߰�

struct SwordAuraTrail
{
    shared_ptr<SwordAuraObject> obj;
    float life = 0.0f; // ��� �ð�
};

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
    void ClearSceneResources() override;

    void AddCubeCollider(const XMFLOAT3& position, const XMFLOAT3& extents, const FLOAT& rotate = 0.f);

    void RenderShadowPass(const ComPtr<ID3D12GraphicsCommandList>& commandList);

    unordered_map<string, vector<shared_ptr<Monsters>>>& GetMonsterGroups() { return m_monsterGroups; }
    shared_ptr<ParticleManager> GetParticleManager() const { return m_particleManager; }
    void SetGoldScore(int score) { m_goldScore = score; }
    void SetInventoryCount(int item , int num) { m_inventoryCounts[item] = num; }
    void SetupgradeScore(int num) { m_upgradeScore = num; }
    void SetZenithEnabled() { m_ZenithEnabled = true; }

    //���â ����
    void HandleMouseClick(int mouseX, int mouseY);
    void SetWeaponSlotUV(int type);
    void SetJobSlotUV(int type);
    void UpdateEnhanceDigits();

    void SetAttackCollision(bool SAC) { m_AttackCollision = SAC; }
    bool getAttackCollision() { return m_AttackCollision; }

    //��ų
    void SpawnHealingObject(int num);
    void FireMagicBall(); //������ ��Ÿ
    void FireOther1MagicBall(); //������ ��Ÿ
    void FireOther2MagicBall(); //������ ��Ÿ

    void FireUltimateBulletRain(); //������ ��ų
    void FireUltimateBulletRainOther1(); //������ ��ų
    void FireUltimateBulletRainOther2(); //������ ��ų



    void AddTrailObject(const shared_ptr<GameObject>& obj);
    void SpawnMagicImpactEffect(const XMFLOAT3& pos);
    void SpawnHealingEffect(const XMFLOAT3& playerPos);
    void ActivateZenithStageMonsters();// ���� ���� ���� Ȱ��ȭ
    void CheckHealingCollision();
    void ActivateSwordAuraSkill(int num); //���� ��ų
    void UpdateSwordAuraSkill(float timeElapsed);

    //��Ÿ, ��ų ������ �ֽ�ȭ
    void SetskillAttack(int n) { m_skillAttack = n; };
    void SetnormalAttack(int n) { m_normalAttack = n; };

    //Ÿ Ŭ�� ���� ����
    int m_otherPlayerJobs[2] = { 0, 0 }; // 1: ����, 2: ������, 3: ����Ŀ

    void SpawnDustEffect(const XMFLOAT3& pos);
    void SpawnDashWarning(const XMFLOAT3& pos, float yaw); //���� �뽬 ���� ����
    void SpawnShockwaveWarning(const XMFLOAT3& pos); //���� ���� ���� ����
    //�÷��� �ð� ������Ʈ
    void UpdateGameTimeDigits();
	//���� ����
	void SetEnding();
	void SetBossDie(bool end) { m_bossDied = end; };
	void EndingSceneUpdate(float timeElapsed);
	bool GetEndingSceneBool() { return m_showEndingSequence; };
	void SetDanceMotion(bool Dan) { m_OnceDance = Dan; m_OnceDanceAlways = Dan; };
	bool GetDanceMontion() { return m_OnceDanceAlways; };

	//3��Ī ī�޶� ��� ���
    void SetCameraToggle();
    void SetZenithStart(int StartGame) { m_ZenithStartGame = StartGame; };

    vector<shared_ptr<Monsters>> m_bossMonsters; //���� ���� 

    void ChangeJob(int index);
    void SetOtherJob1(int num) { m_OtherJobNum[0] = num; };
    void SetOtherJob2(int num) { m_OtherJobNum[1] = num; };
    //���� �������� ���� ���� ����
    unordered_map<string, vector<shared_ptr<Monsters>>> m_BossStageMonsters; //���� ������������ ���̴� ���� 10����
    int  m_job = 0;
private:
    int m_OtherJobNum[2] = {99,99};

    shared_ptr<FBXLoader> m_fbxLoader; // FBX �δ� �߰�
    shared_ptr<FBXLoader> m_ZenithLoader; // FBX �δ� �߰�
    shared_ptr<FBXLoader> m_playerLoader;
    vector<shared_ptr<MeshBase>> m_fbxMeshes; // FBX���� �ε��� �޽� ����
    vector<shared_ptr<MeshBase>> m_ZenithMeshes; // FBX���� �ε��� �޽� ����
    vector<shared_ptr<GameObject>> m_fbxObjects; // FBX �𵨿� GameObject ����Ʈ �߰�
    vector<shared_ptr<GameObject>> m_ZenithObjects; // ������ ������Ʈ ����


    bool m_debugDrawEnabled = false;
    bool m_ShadowMapEnabled = false;
    
    //������ ���Ű
    bool m_ZenithEnabled = false;


    ////////////////���� ����////////////////
    unordered_map<string, vector<shared_ptr<Monsters>>> m_monsterGroups;
    unordered_map<string, shared_ptr<MeshBase>> m_meshLibrary;



    //�׸��� ����
    shared_ptr<DebugShadowShader> m_debugShadowShader;
    XMMATRIX m_shadowViewMatrix;
    XMMATRIX m_shadowProjMatrix;


    //ī�޶� ��ȯ ����
    CameraMode m_currentCameraMode = CameraMode::QuarterView;
    shared_ptr<QuarterViewCamera> m_quarterViewCamera;
    shared_ptr<ThirdPersonCamera> m_thirdPersonCamera;

    //��� ����
    int m_goldScore = 0; // Gold ����
    vector<shared_ptr<GameObject>> m_goldDigits; // �� �ڸ������� UI ������Ʈ�� ����

    //��ƼŬ ����
    shared_ptr<ParticleManager> m_particleManager;

    //�κ��丮 ���� ����
    vector<shared_ptr<GameObject>> m_inventoryDigits;
    int m_inventoryCounts[6] = { 0, 0, 0, 0, 0, 0 };
    
    //�κ��丮 ���� �� ������ ����
    bool m_WeaponOnly = false;
    bool m_JopOnly = false;

    //��ȭâ ����
    shared_ptr<GameObject> m_reinforcedWindowUI;
    bool m_showReinforcedWindow = false;

    bool m_isReinforceWindowVisible = false; // 'I'Ű ���
    bool m_isReinforceSlotOccupied = false;

    bool m_ZenithStartGame = false;

    string m_selectedItemType = ""; // "weapon", "job"

    shared_ptr<GameObject> m_weaponSlotIcon;
    shared_ptr<GameObject> m_jobSlotIcon;
    shared_ptr<GameObject> m_plusIcon;

    int m_upgradeScore = 0;
    vector<shared_ptr<GameObject>> m_forcedDigits;
    vector<shared_ptr<GameObject>> m_ColonDigit;

    //�� ������ �ܰ��� ���Ű
    bool m_OutLine = false;
    unordered_map<string, unordered_map<string, AnimationClip>> m_animClipLibrary;
    unordered_map<string, unordered_map<int, XMMATRIX>> m_boneOffsetLibrary;
    unordered_map<string, unordered_map<string, int>> m_boneNameMap;
    unordered_map<string, unordered_map<string, string>> m_BoneHierarchy;
    unordered_map<string, unordered_map<string, XMMATRIX>> m_staticNodeTransforms;
    unordered_map<string, unordered_map<string, XMMATRIX>> m_nodeNameToLocalTransform;

    //��ġ �浹
    bool m_AttackCollision = false;
    bool wasKeyPressedF = false; //fŰ �ѹ���

    //��ų
    vector<shared_ptr<GameObject>> m_healingObjects; //���� ������ ������Ʈ
    vector<shared_ptr<HealingEffectObject>> m_healingEffects; //���� ���� �� ����Ʈ

    vector<shared_ptr<GameObject>> m_trailObjects; //������ ��Ÿ Ʈ����

    vector<shared_ptr<MagicBall>> m_magicBalls; // ������ ��Ÿ ��ü
    vector<shared_ptr<MagicBall>> m_OthermagicBalls1; // ������ ��Ÿ ��ü
    vector<shared_ptr<MagicBall>> m_OthermagicBalls2; // ������ ��Ÿ ��ü

    vector<shared_ptr<MagicBall>> m_UltPlayermagicBalls; // ������ ��Ÿ ��ü
    vector<shared_ptr<MagicBall>> m_UltOther1magicBalls; // ������ ��Ÿ ��ü
    vector<shared_ptr<MagicBall>> m_UltOther2magicBalls; // ������ ��Ÿ ��ü


    vector<shared_ptr<GameObject>> m_effects; //������ ��ų, ��Ÿ �ǰݽ� ����Ʈ

    vector<SwordAuraTrail> m_swordAuraTrailList;
    int m_SwordNum = 99; //� Ŭ���� �� ��ų�� �ߵ������

    float m_trailTimer = 0.0f;
    const float TRAIL_SPAWN_INTERVAL = 0.01f; // 50������/�� ���� �ֱ�
    const float TRAIL_LIFETIME = 0.3f;        // 0.3�� �� �����

    float m_magicBasicAttackCooldown = 4.0f;  // ������ ��Ÿ 3�� ��Ÿ��
    float m_magicBasicAttackTimer = 0.0f;     // ��� �ð�
    bool m_magicAttack = false;
    bool m_OnceDance = false; // �� ��Ǹ� ���ϰ� �ϴ� ����
    bool m_OnceDanceAlways = false; // �� ��Ǹ� ���ϰ� �ϴ� ����

    //����
    vector<shared_ptr<GameObject>> m_weopons;


	//���� �װ� ���� ȿ��
	vector<shared_ptr<DissolveDustEffectObject>> m_dustEffects;
    vector<shared_ptr<AttackRangeIndicator>> m_attackIndicators;

    //�ð� ǥ�� �ؽ�Ʈ
    vector<shared_ptr<GameObject>> m_timeDigits;
    vector<shared_ptr<GameObject>> m_skillIcons;
    vector<float> m_skillCooldowns = { 0.f, 0.f, 0.f }; // ���� ��Ÿ��
    vector<float> m_skillMaxCooldowns = { 4.f, 4.f, 4.f }; // �ִ� ��Ÿ��

    vector<shared_ptr<SwordAuraObject>> m_swordAuraObjects;
    bool m_isSwordSkillActive = false;
    float m_swordSkillDuration = 0.0f;
    const float MAX_SWORD_SKILL_DURATION = 10.0f;

    //����ȭ�� ���� ����
    bool m_bossDied = false;       
    bool m_showEndingSequence = false;
    float m_endingTimer = 0.f;
    const float MAX_ENDING_TIME = 2.0f; // �÷��̾ ������ ��
    bool m_moveTimeUI = false;
    float m_timeUIMoveTimer = 0.f;
    const float MAX_TIMEUI_MOVE_DURATION = 2.0f;
    float m_gameStartTime = 0.0f; //���� �ð� ����� ����


    vector<XMFLOAT3> m_timeDigitStartPos;
    vector<XMFLOAT3> m_timeDigitTargetPos;
    XMFLOAT3 m_colonStartPos;
    XMFLOAT3 m_colonTargetPos;
    vector<shared_ptr<GameObject>> m_uiEndingBanner;
    vector<shared_ptr<GameObject>> m_uiPressOn;

    //�÷��̾� ��ų ��Ÿ ������ ����
    int m_skillAttack = 0;
    int m_normalAttack = 0;


    public:
    //���� �� ���� �Ž� ����
    array<shared_ptr<Player>, 3> m_jobPlayers; // 0=����, 1=������, 2=����Ŀ
    array<shared_ptr<OtherPlayer>, 6> m_jobOtherPlayers; // 0=����, 1=������, 2=����Ŀ


};
