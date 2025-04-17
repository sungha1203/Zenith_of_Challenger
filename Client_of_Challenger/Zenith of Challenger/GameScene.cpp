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


		for (auto& monster : m_Monsters)
		{
			if (monster) monster->SetDrawBoundingBox(m_debugDrawEnabled);
		}

		//for (auto& obj : m_fbxObjects)
		//	obj->SetDrawBoundingBox(m_debugDrawEnabled);

		OutputDebugStringA(m_debugDrawEnabled ? "[디버그] AABB ON\n" : "[디버그] AABB OFF\n");
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

	for (auto& object : m_objects) {
		object->Update(timeElapsed);
	}
	m_skybox->SetPosition(m_camera->GetEye());

	// 플레이어 위치 가져오기
	if (gGameFramework->GetPlayer())
	{
		const XMFLOAT3& playerPos = gGameFramework->GetPlayer()->GetPosition();
	}

	for (auto& monster : m_Monsters)
		monster->Update(timeElapsed);


	// 충돌 테스트
	auto playerBox = m_player->GetBoundingBox();

	for (auto& monster : m_Monsters)
	{
		auto monsterBox = monster->GetBoundingBox();
		auto monsterCenter = monsterBox.Center;

		// 월드 위치 적용
		XMFLOAT3 playerWorldPos = m_player->GetPosition();
		XMFLOAT3 monsterWorldPos = monster->GetPosition();

		// 센터 기준 실제 위치 계산 (로컬 center 기준일 경우 반영)
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

		// 거리 기반 충돌 체크 (AABB Extents 포함)
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
			OutputDebugStringA("[충돌 감지] 플레이어와 몬스터 충돌!\n");

			// 예: 충돌 시 몬스터를 붉은색으로 표시
			monster->SetBaseColor(XMFLOAT4(1.f, 0.f, 0.f, 1.f));
		}
		else
		{
			// 충돌 안 한 경우 원래 색
			monster->SetBaseColor(XMFLOAT4(1.f, 1.f, 1.f, 1.f));
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

	if (!m_Monsters.empty())
	{
		for (const auto& monster : m_Monsters)
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
			//	/* RootParameterIndex::UIFillAmount */ 1, 1, &healthRatio, 0);

			ui->Render(commandList);
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

		// [4] 위치 및 스케일 설정
		//player->SetPosition(XMFLOAT3{ -185.f, 53.f, 177.f });
		player->SetPosition(gGameFramework->g_pos);

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
		playerBox.Center = XMFLOAT3{ 0.f, 5.0f, 0.f };
		playerBox.Extents = { 1.0f, 4.5f, 1.0f }; // 스케일링된 값
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

	//몬스터 로드
	LoadAllMonsters(device, m_textures, m_shaders, m_Monsters);

	// 배치
	for (int i = 0; i < m_Monsters.size(); ++i)
	{
		// 위치 분산 (타원형 형태로 배치)
		float angle = XM_2PI * i / m_Monsters.size();
		float radius = 15.0f;
		float x = -135.f + radius * cos(angle);
		float z = 130.f + radius * sin(angle);
		float y = 48.f;

		m_Monsters[i]->SetPosition(XMFLOAT3{ x, y, z });
	}


	m_skybox = make_shared<GameObject>(device);
	m_skybox->SetMesh(m_meshes["SKYBOX"]);
	m_skybox->SetTextureIndex(m_textures["SKYBOX"]->GetTextureIndex());
	m_skybox->SetTexture(m_textures["SKYBOX"]);
	m_skybox->SetShader(m_shaders["SKYBOX"]);


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
}