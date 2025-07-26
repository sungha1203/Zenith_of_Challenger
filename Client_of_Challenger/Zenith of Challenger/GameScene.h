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

class FBXLoader; // 전방 선언 추가

struct SwordAuraTrail
{
    shared_ptr<SwordAuraObject> obj;
    float life = 0.0f; // 경과 시간
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

    //장비창 관련
    void HandleMouseClick(int mouseX, int mouseY);
    void SetWeaponSlotUV(int type);
    void SetJobSlotUV(int type);
    void UpdateEnhanceDigits();

    void SetAttackCollision(bool SAC) { m_AttackCollision = SAC; }
    bool getAttackCollision() { return m_AttackCollision; }

    //스킬
    void SpawnHealingObject(int num);
    void FireMagicBall(); //마법사 평타
    void FireOther1MagicBall(); //마법사 평타
    void FireOther2MagicBall(); //마법사 평타

    void FireUltimateBulletRain(); //마법사 스킬
    void FireUltimateBulletRainOther1(); //마법사 스킬
    void FireUltimateBulletRainOther2(); //마법사 스킬



    void AddTrailObject(const shared_ptr<GameObject>& obj);
    void SpawnMagicImpactEffect(const XMFLOAT3& pos);
    void SpawnHealingEffect(const XMFLOAT3& playerPos);
    void ActivateZenithStageMonsters();// 몬스터 렌더 여부 활성화
    void CheckHealingCollision();
    void ActivateSwordAuraSkill(int num); //전사 스킬
    void UpdateSwordAuraSkill(float timeElapsed);

    //평타, 스킬 데미지 최신화
    void SetskillAttack(int n) { m_skillAttack = n; };
    void SetnormalAttack(int n) { m_normalAttack = n; };

    //타 클라 직업 판정
    int m_otherPlayerJobs[2] = { 0, 0 }; // 1: 전사, 2: 마법사, 3: 힐탱커

    void SpawnDustEffect(const XMFLOAT3& pos);
    void SpawnDashWarning(const XMFLOAT3& pos, float yaw); //보스 대쉬 공격 범위
    void SpawnShockwaveWarning(const XMFLOAT3& pos); //보스 점프 공격 범위
    //플레이 시간 업데이트
    void UpdateGameTimeDigits();
	//엔딩 관련
	void SetEnding();
	void SetBossDie(bool end) { m_bossDied = end; };
	void EndingSceneUpdate(float timeElapsed);
	bool GetEndingSceneBool() { return m_showEndingSequence; };
	void SetDanceMotion(bool Dan) { m_OnceDance = Dan; m_OnceDanceAlways = Dan; };
	bool GetDanceMontion() { return m_OnceDanceAlways; };

	//3인칭 카메라 모드 토글
    void SetCameraToggle();
    void SetZenithStart(int StartGame) { m_ZenithStartGame = StartGame; };

    vector<shared_ptr<Monsters>> m_bossMonsters; //보스 몬스터 

    void ChangeJob(int index);
    void SetOtherJob1(int num) { m_OtherJobNum[0] = num; };
    void SetOtherJob2(int num) { m_OtherJobNum[1] = num; };
    //정점 스테이지 몬스터 관리 변수
    unordered_map<string, vector<shared_ptr<Monsters>>> m_BossStageMonsters; //정점 스테이지에서 쓰이는 몬스터 10마리
    int  m_job = 0;
private:
    int m_OtherJobNum[2] = {99,99};

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
    shared_ptr<DebugShadowShader> m_debugShadowShader;
    XMMATRIX m_shadowViewMatrix;
    XMMATRIX m_shadowProjMatrix;


    //카메라 전환 관련
    CameraMode m_currentCameraMode = CameraMode::QuarterView;
    shared_ptr<QuarterViewCamera> m_quarterViewCamera;
    shared_ptr<ThirdPersonCamera> m_thirdPersonCamera;

    //골드 관련
    int m_goldScore = 0; // Gold 점수
    vector<shared_ptr<GameObject>> m_goldDigits; // 각 자릿수마다 UI 오브젝트를 저장

    //파티클 관련
    shared_ptr<ParticleManager> m_particleManager;

    //인벤토리 숫자 관련
    vector<shared_ptr<GameObject>> m_inventoryDigits;
    int m_inventoryCounts[6] = { 0, 0, 0, 0, 0, 0 };
    
    //인벤토리 무기 및 전직서 관련
    bool m_WeaponOnly = false;
    bool m_JopOnly = false;

    //강화창 관련
    shared_ptr<GameObject> m_reinforcedWindowUI;
    bool m_showReinforcedWindow = false;

    bool m_isReinforceWindowVisible = false; // 'I'키 토글
    bool m_isReinforceSlotOccupied = false;

    bool m_ZenithStartGame = false;

    string m_selectedItemType = ""; // "weapon", "job"

    shared_ptr<GameObject> m_weaponSlotIcon;
    shared_ptr<GameObject> m_jobSlotIcon;
    shared_ptr<GameObject> m_plusIcon;

    int m_upgradeScore = 0;
    vector<shared_ptr<GameObject>> m_forcedDigits;
    vector<shared_ptr<GameObject>> m_ColonDigit;

    //툰 렌더링 외곽선 토글키
    bool m_OutLine = false;
    unordered_map<string, unordered_map<string, AnimationClip>> m_animClipLibrary;
    unordered_map<string, unordered_map<int, XMMATRIX>> m_boneOffsetLibrary;
    unordered_map<string, unordered_map<string, int>> m_boneNameMap;
    unordered_map<string, unordered_map<string, string>> m_BoneHierarchy;
    unordered_map<string, unordered_map<string, XMMATRIX>> m_staticNodeTransforms;
    unordered_map<string, unordered_map<string, XMMATRIX>> m_nodeNameToLocalTransform;

    //펀치 충돌
    bool m_AttackCollision = false;
    bool wasKeyPressedF = false; //f키 한번만

    //스킬
    vector<shared_ptr<GameObject>> m_healingObjects; //힐링 아이템 오브젝트
    vector<shared_ptr<HealingEffectObject>> m_healingEffects; //힐팩 습득 시 이펙트

    vector<shared_ptr<GameObject>> m_trailObjects; //마법사 평타 트레일

    vector<shared_ptr<MagicBall>> m_magicBalls; // 마법사 평타 구체
    vector<shared_ptr<MagicBall>> m_OthermagicBalls1; // 마법사 평타 구체
    vector<shared_ptr<MagicBall>> m_OthermagicBalls2; // 마법사 평타 구체

    vector<shared_ptr<MagicBall>> m_UltPlayermagicBalls; // 마법사 평타 구체
    vector<shared_ptr<MagicBall>> m_UltOther1magicBalls; // 마법사 평타 구체
    vector<shared_ptr<MagicBall>> m_UltOther2magicBalls; // 마법사 평타 구체


    vector<shared_ptr<GameObject>> m_effects; //마법사 스킬, 평타 피격시 이펙트

    vector<SwordAuraTrail> m_swordAuraTrailList;
    int m_SwordNum = 99; //어떤 클라의 검 스킬이 발동됬는지

    float m_trailTimer = 0.0f;
    const float TRAIL_SPAWN_INTERVAL = 0.01f; // 50프레임/초 생성 주기
    const float TRAIL_LIFETIME = 0.3f;        // 0.3초 후 사라짐

    float m_magicBasicAttackCooldown = 4.0f;  // 마법사 평타 3초 쿨타임
    float m_magicBasicAttackTimer = 0.0f;     // 경과 시간
    bool m_magicAttack = false;
    bool m_OnceDance = false; // 댄스 모션만 취하게 하는 변수
    bool m_OnceDanceAlways = false; // 댄스 모션만 취하게 하는 변수

    //무기
    vector<shared_ptr<GameObject>> m_weopons;


	//보스 죽고 먼지 효과
	vector<shared_ptr<DissolveDustEffectObject>> m_dustEffects;
    vector<shared_ptr<AttackRangeIndicator>> m_attackIndicators;

    //시간 표시 텍스트
    vector<shared_ptr<GameObject>> m_timeDigits;
    vector<shared_ptr<GameObject>> m_skillIcons;
    vector<float> m_skillCooldowns = { 0.f, 0.f, 0.f }; // 남은 쿨타임
    vector<float> m_skillMaxCooldowns = { 4.f, 4.f, 4.f }; // 최대 쿨타임

    vector<shared_ptr<SwordAuraObject>> m_swordAuraObjects;
    bool m_isSwordSkillActive = false;
    float m_swordSkillDuration = 0.0f;
    const float MAX_SWORD_SKILL_DURATION = 10.0f;

    //엔딩화면 전용 변수
    bool m_bossDied = false;       
    bool m_showEndingSequence = false;
    float m_endingTimer = 0.f;
    const float MAX_ENDING_TIME = 2.0f; // 플레이어가 도달할 시
    bool m_moveTimeUI = false;
    float m_timeUIMoveTimer = 0.f;
    const float MAX_TIMEUI_MOVE_DURATION = 2.0f;
    float m_gameStartTime = 0.0f; //시작 시간 저장용 변수


    vector<XMFLOAT3> m_timeDigitStartPos;
    vector<XMFLOAT3> m_timeDigitTargetPos;
    XMFLOAT3 m_colonStartPos;
    XMFLOAT3 m_colonTargetPos;
    vector<shared_ptr<GameObject>> m_uiEndingBanner;
    vector<shared_ptr<GameObject>> m_uiPressOn;

    //플레이어 스킬 평타 데미지 저장
    int m_skillAttack = 0;
    int m_normalAttack = 0;


    public:
    //전직 별 직업 매쉬 정보
    array<shared_ptr<Player>, 3> m_jobPlayers; // 0=전사, 1=마법사, 2=힐탱커
    array<shared_ptr<OtherPlayer>, 6> m_jobOtherPlayers; // 0=전사, 1=마법사, 2=힐탱커


};
