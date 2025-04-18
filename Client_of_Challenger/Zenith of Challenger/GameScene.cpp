#include "GameScene.h"
#include "monster.h"
#include "MonsterLoader.h"


void GameScene::BuildObjects(const ComPtr<ID3D12Device>& device,
    const ComPtr<ID3D12GraphicsCommandList>& commandList,
    const ComPtr<ID3D12RootSignature>& rootSignature)
{
    m_meshes.clear();
    m_textures.clear();
    m_objects.clear();
    m_fbxMeshes.clear(); // FBX 메쉬 초기화


    BuildShaders(device, commandList, rootSignature);
    BuildMeshes(device, commandList);
    BuildTextures(device, commandList);
    BuildMaterials(device, commandList);

    // FBX 파일 로드
    cout << "도전 맵 로드 중!!!!" << endl;
    m_fbxLoader = make_shared<FBXLoader>();
    if (m_fbxLoader->LoadFBXModel("Model/Challenge.fbx",
        XMMatrixScaling(5.0f, 5.0f, 5.0f) * XMMatrixRotationY(XMConvertToRadians(180.0f))))
    {
        m_fbxMeshes = m_fbxLoader->GetMeshes();
    }

    // FBX 모델을 GameObject로 변환 후 m_fbxObjects에 추가
    for (const auto& fbxMesh : m_fbxMeshes)
    {
        auto gameObject = make_shared<GameObject>(device);
        gameObject->SetMesh(fbxMesh);
        gameObject->SetScale(XMFLOAT3{ 0.01f, 0.01f, 0.01f }); // 원래 크기로 유지
        gameObject->SetPosition(XMFLOAT3{ 0.0f, 0.0f, 0.0f }); // Y축 위치 조정

        // 여기서 AABB 생성 (정적 오브젝트는 1회만)
        BoundingBox box;
        box.Center = gameObject->GetPosition();
        box.Extents = { 1.0f, 1.0f, 1.0f }; // 적절한 값으로 조절

        gameObject->SetBoundingBox(box);

        m_fbxObjects.push_back(gameObject);
    }

    BuildObjects(device);
}

void GameScene::MouseEvent(HWND hWnd, FLOAT timeElapsed)
{
}

void GameScene::KeyboardEvent(FLOAT timeElapsed)
{
    m_player->KeyboardEvent(timeElapsed);

    if (GetAsyncKeyState(VK_F1) & 0x0001)
    {
        m_debugDrawEnabled = !m_debugDrawEnabled;

        if (m_player) m_player->SetDrawBoundingBox(m_debugDrawEnabled);


        for (auto& [type, group] : m_monsterGroups)
        {
            for (auto& monster : group)
            {
                if (monster) monster->SetDrawBoundingBox(m_debugDrawEnabled);
            }
        }

        OutputDebugStringA(m_debugDrawEnabled ? "[Debug] AABB ON\n" : "[Debug] AABB OFF\n");
    }
    if (GetAsyncKeyState(VK_LEFT) & 0x8000)
        m_uiObjects[1]->m_fillAmount -= 0.1;

    if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
        m_uiObjects[1]->m_fillAmount += 0.1;
}

void GameScene::Update(FLOAT timeElapsed)
{
    m_player->Update(timeElapsed);
    m_sun->Update(timeElapsed);

    for (auto& object : m_objects)
        object->Update(timeElapsed);

    m_skybox->SetPosition(m_camera->GetEye());

    // 플레이어 위치 가져오기
    if (gGameFramework->GetPlayer())
    {
        const XMFLOAT3& playerPos = gGameFramework->GetPlayer()->GetPosition();
    }

    // [1] 몬스터 업데이트 (map 기반)
    for (auto& [type, group] : m_monsterGroups)
    {
        for (auto& monster : group)
            monster->Update(timeElapsed);
    }

    // [2] 충돌 테스트
    auto playerBox = m_player->GetBoundingBox();

    if (playerBox.Extents.x == 0.f && playerBox.Extents.y == 0.f && playerBox.Extents.z == 0.f)
        return;

    for (auto& [type, group] : m_monsterGroups)
    {
        for (auto& monster : group)
        {
            auto monsterBox = monster->GetBoundingBox();
            auto monsterCenter = monsterBox.Center;

            XMFLOAT3 playerWorldPos = m_player->GetPosition();
            XMFLOAT3 monsterWorldPos = monster->GetPosition();

            XMFLOAT3 playerCenterWorld = {
               playerWorldPos.x,
               playerWorldPos.y + 5.0f,
               playerWorldPos.z
            };

            XMFLOAT3 monsterCenterWorld = {
               monsterWorldPos.x + monsterCenter.x,
               monsterWorldPos.y + monsterCenter.y,
               monsterWorldPos.z + monsterCenter.z
            };

            float dx = abs(playerCenterWorld.x - monsterCenterWorld.x);
            float dy = abs(playerCenterWorld.y - monsterCenterWorld.y);
            float dz = abs(playerCenterWorld.z - monsterCenterWorld.z);

            const XMFLOAT3& playerExtent = playerBox.Extents;
            const XMFLOAT3& monsterExtent = monsterBox.Extents;

            bool intersectX = dx <= (playerExtent.x + monsterExtent.x);
            bool intersectY = dy <= (playerExtent.y + monsterExtent.y);
            bool intersectZ = dz <= (playerExtent.z + monsterExtent.z);

            if (intersectX && intersectY && intersectZ)
            {
                OutputDebugStringA("[Collision Detection]Players and monsters collide!\n");
                m_player->SetPosition(m_player->m_prevPosition); // 이동 되돌리기
                monster->SetBaseColor(XMFLOAT4(1.f, 0.f, 0.f, 1.f)); // 충돌 시 빨강
            }
            else
            {
                monster->SetBaseColor(XMFLOAT4(1.f, 1.f, 1.f, 1.f)); // 기본 흰색
            }
        }
    }

    for (auto& obj : m_objects)
    {
        const BoundingBox& objBox = obj->GetBoundingBox();
        const XMFLOAT3& objCenter = objBox.Center;

        XMFLOAT3 objPos = obj->GetPosition();
        XMFLOAT3 objCenterWorld = {
            objPos.x + objCenter.x,
            objPos.y + objCenter.y,
            objPos.z + objCenter.z
        };

        XMFLOAT3 playerPos = m_player->GetPosition();
        XMFLOAT3 playerCenterWorld = {
            playerPos.x,
            playerPos.y + 5.0f,
            playerPos.z
        };

        const XMFLOAT3& objExtent = objBox.Extents;

        bool intersectX = abs(playerCenterWorld.x - objCenterWorld.x) <= (playerBox.Extents.x + objExtent.x);
        bool intersectY = abs(playerCenterWorld.y - objCenterWorld.y) <= (playerBox.Extents.y + objExtent.y);
        bool intersectZ = abs(playerCenterWorld.z - objCenterWorld.z) <= (playerBox.Extents.z + objExtent.z);

        if (intersectX && intersectY && intersectZ)
        {
            OutputDebugStringA("[Collision Detection] Player <-> Object\n");
            m_player->SetPosition(m_player->m_prevPosition);
        }
    }

}

void GameScene::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
    commandList->SetGraphicsRootSignature(gGameFramework->GetRootSignature().Get());

    m_camera->UpdateShaderVariable(commandList);
    m_lightSystem->UpdateShaderVariable(commandList);

    m_terrain->Render(commandList);

    m_skybox->Render(commandList);

    if (!m_fbxObjects.empty())
    {
        for (const auto& obj : m_fbxObjects)
            obj->Render(commandList);
    }


    if (m_player)
    {
        m_player->Render(commandList);
    }

    // 몬스터 렌더링 (map 기반으로 수정)
    for (const auto& [type, group] : m_monsterGroups)
    {
        for (const auto& monster : group)
        {
            monster->Render(commandList);
        }
    }

    if (!m_uiObjects.empty())
    {
        m_shaders.at("UI")->UpdateShaderVariable(commandList);

        float healthRatio = 1.0f/*m_player->GetCurrentHP() / m_player->GetMaxHP()*/;

        for (const auto& ui : m_uiObjects)
        {
            // b1 슬롯에 체력비율 전달 (셰이더에서 g_fillAmount로 사용됨)
            //commandList->SetGraphicsRoot32BitConstants(
            //   /* RootParameterIndex::UIFillAmount */ 1, 1, &healthRatio, 0);

            ui->Render(commandList);
        }
    }

    // 충돌체크용 일반 오브젝트 렌더링
    if (!m_objects.empty() && m_debugDrawEnabled == true)
    {
        for (const auto& object : m_objects)
        {
            object->Render(commandList);
        }
    }

}



void GameScene::PreRender(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    ID3D12DescriptorHeap* heaps[] = { gGameFramework->GetDescriptorHeap().Get() };
    commandList->SetDescriptorHeaps(_countof(heaps), heaps);
}

void GameScene::BuildShaders(const ComPtr<ID3D12Device>& device,
    const ComPtr<ID3D12GraphicsCommandList>& commandList,
    const ComPtr<ID3D12RootSignature>& rootSignature)
{
    auto objectShader = make_shared<ObjectShader>(device, rootSignature);
    m_shaders.insert({ "OBJECT", objectShader });
    auto skyboxShader = make_shared<SkyboxShader>(device, rootSignature);
    m_shaders.insert({ "SKYBOX", skyboxShader });
    auto detailShader = make_shared<DetailShader>(device, rootSignature);
    m_shaders.insert({ "DETAIL", detailShader });
    //FBX 전용 쉐이더 추가
    auto fbxShader = make_shared<FBXShader>(device, rootSignature);
    m_shaders.insert({ "FBX", fbxShader });
    auto uiShader = make_shared<GameSceneUIShader>(device, rootSignature);
    m_shaders.insert({ "UI", uiShader });
    // Character 애니메이션 전용 셰이더 추가
    auto characterShader = make_shared<CharacterShader>(device, rootSignature);
    m_shaders.insert({ "CHARACTER", characterShader });
    // 몬스터 (날파리) 전용 셰이더 추가
    auto frightFlyShader = make_shared<FrightFlyShader>(device, rootSignature);
    m_shaders.insert({ "FrightFly", frightFlyShader });

    auto debugLineShader = make_shared<DebugLineShader>(device, rootSignature);
    m_shaders.insert({ "DebugLineShader", debugLineShader });
}

void GameScene::BuildMeshes(const ComPtr<ID3D12Device>& device,
    const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    auto cubeMesh = make_shared<Mesh<TextureVertex>>(device, commandList,
        TEXT("Model/CubeNormalMesh.binary"));
    m_meshes.insert({ "CUBE", cubeMesh });
    auto skyboxMesh = make_shared<Mesh<Vertex>>(device, commandList,
        TEXT("Model/SkyboxMesh.binary"));
    m_meshes.insert({ "SKYBOX", skyboxMesh });
    auto terrainMesh = make_shared<TerrainMesh>(device, commandList,
        TEXT("Model/HeightMap.raw"));
    m_meshes.insert({ "TERRAIN", terrainMesh });
    // Frightfly FBX 메쉬 저장
    auto frightflyLoader = make_shared<FBXLoader>();
    if (frightflyLoader->LoadFBXModel("Model/Monsters/Frightfly/Frightfly_01.fbx", XMMatrixScaling(0.05f, 0.05f, 0.05f)))
    {
        auto meshes = frightflyLoader->GetMeshes();
        if (!meshes.empty())
        {
            m_meshLibrary["Frightfly"] = meshes[0]; // 여러 개면 처리 필요

            // 애니메이션 정보도 캐싱
            const auto& clips = frightflyLoader->GetAnimationClips();
            for (auto& clip : clips)
                m_animClipLibrary[clip.name] = clip;

            m_boneOffsetLibrary = frightflyLoader->GetBoneOffsets();
            m_boneNameMap = frightflyLoader->GetBoneNameToIndex();
        }
    }
    // Flower_Fairy FBX 메쉬 저장
    auto flowerFairyLoader = make_shared<FBXLoader>();
    if (flowerFairyLoader->LoadFBXModel("Model/Monsters/Flower_Fairy/Flower_Fairy.fbx", XMMatrixScaling(0.1f, 0.1f, 0.1f)))
    {
        auto meshes = flowerFairyLoader->GetMeshes();
        if (!meshes.empty())
        {
            m_meshLibrary["Flower_Fairy"] = meshes[0];

            const auto& clips = flowerFairyLoader->GetAnimationClips();
            for (auto& clip : clips)
                m_animClipLibrary[clip.name] = clip;

            m_boneOffsetLibrary = flowerFairyLoader->GetBoneOffsets();
            m_boneNameMap = flowerFairyLoader->GetBoneNameToIndex();
        }
        else
        {
            OutputDebugStringA("[FBXLoader] Flower_Fairy 메쉬 없음\n");
        }
    }
    // Mushroom_Dark FBX 메쉬 저장
    auto mushroomDarkLoader = make_shared<FBXLoader>();
    if (mushroomDarkLoader->LoadFBXModel("Model/Monsters/Mushroom_Dark/Mushroom_Dark.fbx", XMMatrixScaling(0.1f, 0.1f, 0.1f)))
    {
        auto meshes = mushroomDarkLoader->GetMeshes();
        if (!meshes.empty())
        {
            m_meshLibrary["Mushroom_Dark"] = meshes[0];

            const auto& clips = mushroomDarkLoader->GetAnimationClips();
            for (auto& clip : clips)
                m_animClipLibrary[clip.name] = clip;

            m_boneOffsetLibrary = mushroomDarkLoader->GetBoneOffsets();
            m_boneNameMap = mushroomDarkLoader->GetBoneNameToIndex();
        }
        else
        {
            OutputDebugStringA("[FBXLoader] Mushroom_Dark 메쉬 없음\n");
        }
    }
    // Venus_Blue FBX 메쉬 저장
    auto venusBlueLoader = make_shared<FBXLoader>();
    if (venusBlueLoader->LoadFBXModel("Model/Monsters/Venus_Blue/Venus_Blue.fbx", XMMatrixScaling(0.1f, 0.1f, 0.1f)))
    {
        auto meshes = venusBlueLoader->GetMeshes();
        if (!meshes.empty())
        {
            m_meshLibrary["Venus_Blue"] = meshes[0];

            const auto& clips = venusBlueLoader->GetAnimationClips();
            for (auto& clip : clips)
                m_animClipLibrary[clip.name] = clip;

            m_boneOffsetLibrary = venusBlueLoader->GetBoneOffsets();
            m_boneNameMap = venusBlueLoader->GetBoneNameToIndex();
        }
        else
        {
            OutputDebugStringA("[FBXLoader] Venus_Blue 메쉬 없음\n");
        }
    }
    // Plant_Dionaea FBX 메쉬 저장
    auto Plant_DionaeaLoader = make_shared<FBXLoader>();
    if (Plant_DionaeaLoader->LoadFBXModel("Model/Monsters/Plant_Dionaea/Plant_Dionaea.fbx", XMMatrixScaling(0.1f, 0.1f, 0.1f)))
    {
        auto meshes = Plant_DionaeaLoader->GetMeshes();
        if (!meshes.empty())
        {
            m_meshLibrary["Plant_Dionaea"] = meshes[0];

            const auto& clips = Plant_DionaeaLoader->GetAnimationClips();
            for (auto& clip : clips)
                m_animClipLibrary[clip.name] = clip;

            m_boneOffsetLibrary = Plant_DionaeaLoader->GetBoneOffsets();
            m_boneNameMap = Plant_DionaeaLoader->GetBoneNameToIndex();
        }
        else
        {
            OutputDebugStringA("[FBXLoader] Venus_Blue 메쉬 없음\n");
        }
    }

}

void GameScene::BuildTextures(const ComPtr<ID3D12Device>& device,
    const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    auto skyboxTexture = make_shared<Texture>(device, commandList,
        TEXT("Skybox/SkyBox_0.dds"), RootParameter::TextureCube);
    skyboxTexture->CreateShaderVariable(device, true);
    m_textures.insert({ "SKYBOX", skyboxTexture });

    auto terrainTexture = make_shared<Texture>(device);
    terrainTexture->LoadTexture(device, commandList,
        TEXT("Image/Base_Texture.dds"), RootParameter::Texture);
    terrainTexture->LoadTexture(device, commandList,
        TEXT("Image/Detail_Texture_7.dds"), RootParameter::Texture);
    terrainTexture->CreateShaderVariable(device, true);
    m_textures.insert({ "TERRAIN", terrainTexture });

    auto fbxTexture = make_shared<Texture>(device, commandList,
        TEXT("Image/Base Map.dds"), RootParameter::Texture);
    fbxTexture->CreateShaderVariable(device, true);
    m_textures.insert({ "FBX", fbxTexture });

    auto characterTexture = make_shared<Texture>(device, commandList,
        TEXT("Image/Texture_Modular_Characters.dds"), RootParameter::Texture);
    characterTexture->CreateShaderVariable(device, true);
    m_textures.insert({ "CHARACTER", characterTexture });

    auto FrightFlyTexture = make_shared<Texture>(device, commandList,
        TEXT("Image/Monsters/FrightFly_converted.dds"), RootParameter::Texture);
    FrightFlyTexture->CreateShaderVariable(device, true);
    m_textures.insert({ "FrightFly", FrightFlyTexture });

    auto healthBarZeroTexture = make_shared<Texture>(device, commandList,
        TEXT("Image/InGameUI/HealthBarZero_BC3.dds"), RootParameter::Texture);
    healthBarZeroTexture->CreateShaderVariable(device, true);
    m_textures.insert({ "HealthBarZero", healthBarZeroTexture });

    auto healthBarTexture = make_shared<Texture>(device, commandList,
        TEXT("Image/InGameUI/HealthBar_BC3.dds"), RootParameter::Texture);
    healthBarTexture->CreateShaderVariable(device, true);
    m_textures.insert({ "HealthBar", healthBarTexture });

    auto inventoryTexture = make_shared<Texture>(device, commandList,
        TEXT("Image/InGameUI/Inventory.dds"), RootParameter::Texture);
    inventoryTexture->CreateShaderVariable(device, true);
    m_textures.insert({ "Inventory", inventoryTexture });

    auto FlowerFairyTexture = make_shared<Texture>(device, commandList,
        TEXT("Image/Monsters/Flower Fairy Yellow.dds"), RootParameter::Texture);
    FlowerFairyTexture->CreateShaderVariable(device, true);
    m_textures.insert({ "Flower_Fairy", FlowerFairyTexture });

    auto Mushroom_DarkTexture = make_shared<Texture>(device, commandList,
        TEXT("Image/Monsters/Polygonal Mushroom Dark.dds"), RootParameter::Texture);
    Mushroom_DarkTexture->CreateShaderVariable(device, true);
    m_textures.insert({ "Mushroom_Dark", Mushroom_DarkTexture });

    auto Venus_BlueTexture = make_shared<Texture>(device, commandList,
        TEXT("Image/Monsters/Venus_Blue.dds"), RootParameter::Texture);
    Venus_BlueTexture->CreateShaderVariable(device, true);
    m_textures.insert({ "Venus_Blue", Venus_BlueTexture });

    auto Plant_DionaeaTexture = make_shared<Texture>(device, commandList,
        TEXT("Image/Monsters/Plant_Dionaea.dds"), RootParameter::Texture);
    Plant_DionaeaTexture->CreateShaderVariable(device, true);
    m_textures.insert({ "Plant_Dionaea", Plant_DionaeaTexture });
	
	auto PortraitTexture = make_shared<Texture>(device, commandList,
		TEXT("Image/InGameUI/Portrait.dds"), RootParameter::Texture);
	PortraitTexture->CreateShaderVariable(device, true);
	m_textures.insert({ "Portrait", PortraitTexture });

}

void GameScene::BuildMaterials(const ComPtr<ID3D12Device>& device,
    const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    auto cubeMaterial = make_shared<Material>();
    cubeMaterial->SetMaterial(XMFLOAT3{ 0.95f, 0.93f, 0.88f }, 0.125f, XMFLOAT3{ 0.1f, 0.1f, 0.1f });
    cubeMaterial->CreateShaderVariable(device);
    m_materials.insert({ "CUBE", cubeMaterial });
}

void GameScene::BuildObjects(const ComPtr<ID3D12Device>& device)
{
    m_camera = make_shared<QuarterViewCamera>(device);
    m_camera->SetLens(0.25 * XM_PI, gGameFramework->GetAspectRatio(), 0.1f, 1000.f);

    m_lightSystem = make_unique<LightSystem>(device);
    auto sunLight = make_shared<DirectionalLight>();
    m_lightSystem->SetLight(sunLight);

    m_sun = make_unique<Sun>(sunLight);
    m_sun->SetStrength(XMFLOAT3{ 1.3f, 1.3f, 1.3f }); //디렉셔널 라이트 세기 줄이기

    // [1] 플레이어 모델용 스케일 행렬 설정 (크기 조절)
    XMMATRIX playerTransform = XMMatrixScaling(0.05f, 0.05f, 0.05);

    // [2] FBX 로더 생성 및 모델 로드
    m_playerLoader = make_shared<FBXLoader>();
    cout << "캐릭터 로드 중!!!!" << endl;

    if (m_playerLoader->LoadFBXModel("Model/Player/Player2.fbx", playerTransform))
    {
        auto& meshes = m_playerLoader->GetMeshes();
        if (meshes.empty()) {
            OutputDebugStringA("[ERROR] FBX에서 메시를 찾을 수 없습니다.\n");
            return;
        }

        // [3] Player 객체 생성
        auto player = make_shared<Player>(device);


        player->SetScale(XMFLOAT3{ 1.f, 1.f, 1.f }); // 기본값 확정
        player->SetRotationY(0.f);                  // 정면을 보게 초기화

        // [4] 위치 및 스케일 설정
        player->SetPosition(XMFLOAT3{ -180, 0.7f, -185 });
        //player->SetPosition(gGameFramework->g_pos);

        // [5] FBX 메시 전부 등록
        for (int i = 0; i < meshes.size(); ++i)
        {
            player->AddMesh(meshes[i]);
        }

        // [6] 애니메이션 클립 및 본 정보 설정
        player->SetAnimationClips(m_playerLoader->GetAnimationClips());
        player->SetCurrentAnimation("Idle");
        player->SetBoneOffsets(m_playerLoader->GetBoneOffsets());
        player->SetBoneNameToIndex(m_playerLoader->GetBoneNameToIndex());

        // [7] 텍스처, 머티리얼 설정
        player->SetTexture(m_textures["CHARACTER"]);
        player->SetTextureIndex(m_textures["CHARACTER"]->GetTextureIndex());
        player->SetMaterial(m_materials["CHARACTER"]); // 없으면 생성 필요
        player->SetShader(m_shaders["CHARACTER"]); // 없으면 생성 필요
        player->SetDebugLineShader(m_shaders["DebugLineShader"]);

        // m_player 생성 이후 위치
        BoundingBox playerBox;
        playerBox.Center = XMFLOAT3{ 0.f, 4.0f, 0.f };
        playerBox.Extents = { 1.0f, 4.0f, 1.0f }; // 스케일링된 값
        player->SetBoundingBox(playerBox);

        // [8] 본 행렬 StructuredBuffer용 SRV 생성
        auto [cpuHandle, gpuHandle] = gGameFramework->AllocateDescriptorHeapSlot();
        player->CreateBoneMatrixSRV(device, cpuHandle, gpuHandle);

        // [9] Player 등록 및 GameScene 내부에 저장
        gGameFramework->SetPlayer(player);
        m_player = gGameFramework->GetPlayer();
    }
    else
    {
        OutputDebugStringA("[ERROR] 플레이어 FBX 로드 실패!\n");
    }

    m_player->SetCamera(m_camera);


    //맵의 오브젝트들 바운딩 박스

    //AddCubeCollider({ -212, 5, -211 }, { 10, 15, 10 });
    //AddCubeCollider({ -209, 11, -104 }, { 5, 5, 6.5 });

    AddCubeCollider({ -212, 0, -211 }, { 21, 15, 20 });
    AddCubeCollider({ -209, 0, -107 }, { 9, 23, 13 });
    AddCubeCollider({ -200, 0, 70 }, { 20, 15, 20 });
    AddCubeCollider({ -105, 20, -1 }, { 30, 30, 28 });
    AddCubeCollider({ -130, 0, -85 }, { 10, 20, 20 }, 75.f);
    AddCubeCollider({ -157, 0, -103 }, { 10, 15, 10 });
    AddCubeCollider({ 31, 20, -136 }, { 1, 1, 1 });
    AddCubeCollider({ 113, 20, -196 }, { 1, 1, 1 });
    AddCubeCollider({ 133, 20, -223 }, { 1, 1, 1 });
    AddCubeCollider({ 132, 20, -86 }, { 1, 1, 1 });
    AddCubeCollider({ 204, 20, -117 }, { 1, 1, 1 });
    AddCubeCollider({ 225, 20,-90 }, { 1, 1, 1 });
    AddCubeCollider({ 55, 5, 30 }, { 1, 1, 1 });
    AddCubeCollider({ -3, 5, 67 }, { 1, 1, 1 });
    AddCubeCollider({ -81, 5, 118 }, { 1, 1, 1 });
    AddCubeCollider({ 44, 5, 140 }, { 1, 1, 1 });
    AddCubeCollider({ 17, 5, 181 }, { 1, 1, 1 });
    AddCubeCollider({ 107, 5, 152 }, { 1, 1, 1 });





    //몬스터 로드
    LoadAllMonsters(
        device,
        m_textures,
        m_shaders,
        m_meshLibrary,
        m_animClipLibrary,
        m_boneOffsetLibrary,
        m_boneNameMap,
        m_monsterGroups);

    // "Frightfly" 타입 몬스터 배치
    auto& frightflies = m_monsterGroups["Frightfly"];
    for (int i = 0; i < frightflies.size(); ++i)
    {
        float angle = XM_2PI * i / frightflies.size();
        float radius = 15.0f;
        float x = -170.f + radius * cos(angle);
        float z = 15.f + radius * sin(angle);
        float y = 0.f;

        frightflies[i]->SetPosition(XMFLOAT3{ x, y, z });
    }

    // "Flower_Fairy" 타입 몬스터 배치
    auto& fairies = m_monsterGroups["Flower_Fairy"];
    for (int i = 0; i < fairies.size(); ++i)
    {
        float angle = XM_2PI * i / fairies.size();
        float radius = 20.0f;
        float x = 190.f + radius * cos(angle);
        float z = -190.f + radius * sin(angle);
        float y = -5.f;

        fairies[i]->SetPosition(XMFLOAT3{ x, y, z });
    }

    // "Mushroom_Dark" 타입 몬스터 배치
    auto& mushrooms = m_monsterGroups["Mushroom_Dark"];
    for (int i = 0; i < mushrooms.size(); ++i)
    {
        float angle = XM_2PI * i / mushrooms.size();
        float radius = 25.0f; // 조금 더 넓게 배치
        float x = -100.f + radius * cos(angle);
        float z = -165.f + radius * sin(angle);
        float y = 0.f; // 지면 높이에 맞게 조절

        mushrooms[i]->SetPosition(XMFLOAT3{ x, y, z });
    }

    // "Venus_Blue" 타입 몬스터 배치
    auto& venusGroup = m_monsterGroups["Venus_Blue"];
    for (int i = 0; i < venusGroup.size(); ++i)
    {
        float angle = XM_2PI * i / venusGroup.size();
        float radius = 28.0f; // 위치 조정
        float x = 40.f + radius * cos(angle);
        float z = -50.f + radius * sin(angle);
        float y = 0.f;

        venusGroup[i]->SetPosition(XMFLOAT3{ x, y, z });
    }

    // "Plant_Dionaea" 타입 몬스터 배치
    auto& DionaeaGroup = m_monsterGroups["Plant_Dionaea"];
    for (int i = 0; i < DionaeaGroup.size(); ++i)
    {
        float angle = XM_2PI * i / DionaeaGroup.size();
        float radius = 28.0f; // 위치 조정
        float x = 160.f + radius * cos(angle);
        float z = 30.f + radius * sin(angle);
        float y = 0.f;

        DionaeaGroup[i]->SetPosition(XMFLOAT3{ x, y, z });
    }

    //스카이박스
    m_skybox = make_shared<GameObject>(device);
    m_skybox->SetMesh(m_meshes["SKYBOX"]);
    m_skybox->SetTextureIndex(m_textures["SKYBOX"]->GetTextureIndex());
    m_skybox->SetTexture(m_textures["SKYBOX"]);
    m_skybox->SetShader(m_shaders["SKYBOX"]);

    //터레인
    m_terrain = make_shared<Terrain>(device);
    m_terrain->SetMesh(m_meshes["TERRAIN"]);
    m_terrain->SetTextureIndex(m_textures["TERRAIN"]->GetTextureIndex());
    m_terrain->SetTexture(m_textures["TERRAIN"]);
    m_terrain->SetShader(m_shaders["DETAIL"]);
    m_terrain->SetScale(XMFLOAT3{ 5.f, 0.25f, 5.f });
    m_terrain->SetPosition(XMFLOAT3{ 0.f, -100.f, 0.f });


    for (auto& obj : m_fbxObjects)
    {
        obj->SetTexture(m_textures["FBX"]);
        obj->SetTextureIndex(m_textures["FBX"]->GetTextureIndex());
        obj->SetShader(m_shaders["FBX"]);
        obj->SetUseTexture(true); // UV 기반 텍스처 적용
    }
    auto healthBarZeroUI = make_shared<GameObject>(device);

    healthBarZeroUI->SetTexture(m_textures["HealthBarZero"]);  // 우리가 방금 로드한 텍스처 사용  
    healthBarZeroUI->SetTextureIndex(m_textures["HealthBarZero"]->GetTextureIndex());  //   
    healthBarZeroUI->SetMesh(CreateScreenQuad(device, gGameFramework->GetCommandList(), 1.4f, 0.15f, 0.98f));
    //healthBarUI->SetPosition({0.f, -0.6f, -0.85f });        // NDC 좌표로 하단 왼쪽 고정 (롤 스타일)
    //healthBarUI->SetPosition(XMFLOAT3(0.f, 0.2f, 0.98f));        // NDC 좌표로 하단 왼쪽 고정 (롤 스타일)
    healthBarZeroUI->SetPosition(XMFLOAT3(-0.3f, -0.7f, 0.98f));
    healthBarZeroUI->SetScale(XMFLOAT3(1.2f, 1.2f, 1.2f));
    healthBarZeroUI->SetUseTexture(true);
    healthBarZeroUI->SetBaseColor(XMFLOAT4(1, 1, 1, 1));

    m_uiObjects.push_back(healthBarZeroUI);

    auto healthBarUI = make_shared<GameObject>(device);

    healthBarUI->SetTexture(m_textures["HealthBar"]);  // 우리가 방금 로드한 텍스처 사용
    healthBarUI->SetTextureIndex(m_textures["HealthBar"]->GetTextureIndex());  // 
    healthBarUI->SetMesh(CreateScreenQuad(device, gGameFramework->GetCommandList(), 1.4f, 0.15f, 0.98f));
    //healthBarUI->SetPosition({0.f, -0.6f, -0.85f });        // NDC 좌표로 하단 왼쪽 고정 (롤 스타일)
    //healthBarUI->SetPosition(XMFLOAT3(0.f, 0.2f, 0.98f));        // NDC 좌표로 하단 왼쪽 고정 (롤 스타일)
    healthBarUI->SetPosition(XMFLOAT3(-0.3f, -0.7f, 0.98f));
    healthBarUI->SetScale(XMFLOAT3(1.2f, 1.2f, 1.2f));
    healthBarUI->SetUseTexture(true);
    healthBarUI->SetBaseColor(XMFLOAT4(1, 1, 1, 1));

    m_uiObjects.push_back(healthBarUI);

    auto Inventory = make_shared<GameObject>(device);

    Inventory->SetTexture(m_textures["Inventory"]);  // 우리가 방금 로드한 텍스처 사용
    Inventory->SetTextureIndex(m_textures["Inventory"]->GetTextureIndex());  // 
    Inventory->SetMesh(CreateScreenQuad(device, gGameFramework->GetCommandList(), 0.5f, 0.5f, 0.98f));
    Inventory->SetPosition(XMFLOAT3(0.8f, -0.55f, 0.98f));
    Inventory->SetUseTexture(true);
    Inventory->SetBaseColor(XMFLOAT4(1, 1, 1, 1));

	m_uiObjects.push_back(Inventory);


	auto Portrait = make_shared<GameObject>(device);

	Portrait->SetTexture(m_textures["Portrait"]);  // 우리가 방금 로드한 텍스처 사용
	Portrait->SetTextureIndex(m_textures["Portrait"]->GetTextureIndex());  // 
	Portrait->SetMesh(CreateScreenQuad(device, gGameFramework->GetCommandList(), 0.25f, 0.25f, 0.98f));
	Portrait->SetPosition(XMFLOAT3(-0.9f, -0.4f, 0.98f));
	Portrait->SetUseTexture(true);
	Portrait->SetBaseColor(XMFLOAT4(1, 1, 1, 1));

	m_uiObjects.push_back(Portrait);

}

void GameScene::AddCubeCollider(const XMFLOAT3& position, const XMFLOAT3& extents, const FLOAT& rotate)
{
    auto cube = make_shared<GameObject>(gGameFramework->GetDevice());

    // 메시: 미리 로드된 CUBE 메시 사용
    cube->SetMesh(m_meshes.at("CUBE"));

    // 위치 및 스케일 설정
    cube->SetScale(XMFLOAT3{ 1.f, 1.f, 1.f }); // extents는 반지름이라 *2 필요
    cube->SetRotationY(rotate);
    cube->SetPosition(position);

    // 바운딩 박스 설정
    BoundingBox box;
	box.Center = { 0.f, 0.f, 0.f };
    box.Extents = extents;
    cube->SetBoundingBox(box);

    // 와이어프레임 디버깅
    cube->SetDrawBoundingBox(true);
    cube->SetDebugLineShader(m_shaders.at("DebugLineShader")); // 반드시 등록돼야 함

    // 색상만으로 표현할 경우
    cube->SetBaseColor(XMFLOAT4(1, 0, 0, 1)); // 빨간색 등 원하는 색

    m_objects.push_back(cube); // 오브젝트 등록
}
