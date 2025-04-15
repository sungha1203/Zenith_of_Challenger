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
}

void GameScene::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	commandList->SetGraphicsRootSignature(gGameFramework->GetRootSignature().Get());

	m_camera->UpdateShaderVariable(commandList);
	m_lightSystem->UpdateShaderVariable(commandList);

	m_shaders.at("DETAIL")->UpdateShaderVariable(commandList);
	m_terrain->Render(commandList);

	m_shaders.at("SKYBOX")->UpdateShaderVariable(commandList);
	m_skybox->Render(commandList);

	if (!m_fbxObjects.empty())
	{
		m_shaders.at("FBX")->UpdateShaderVariable(commandList);
		for (const auto& obj : m_fbxObjects)
			obj->Render(commandList);
	}

	if (m_player)
	{
		m_shaders.at("CHARACTER")->UpdateShaderVariable(commandList);
		m_player->Render(commandList);
	}

	if (!m_Monsters.empty())
	{
		m_shaders.at("CHARACTER")->UpdateShaderVariable(commandList); // 캐릭터 쉐이더 재사용

		for (const auto& monster : m_Monsters)
		{
			monster->Render(commandList);
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
	auto uiShader = make_shared<UIScreenShader>(device, rootSignature);
	m_shaders.insert({ "UI", uiShader });
	// Character 애니메이션 전용 셰이더 추가
	auto characterShader = make_shared<CharacterShader>(device, rootSignature);
	m_shaders.insert({ "CHARACTER", characterShader });
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

	//auto characterTexture = make_shared<Texture>(device, commandList,
	//	TEXT("Image/Monsters/FrightFly_01.dds"), RootParameter::Texture);
	//characterTexture->CreateShaderVariable(device, true);
	//m_textures.insert({ "FrightFly", characterTexture });

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

		// [서버 임시]@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ 지환이한테 물어보자
		//gGameFramework->SetPlayer(player);

		// [4] 위치 및 스케일 설정
		player->SetPosition(XMFLOAT3{ -185.f, 53.f, 177.f });

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
		player->SetMaterial(m_materials["CHARACTER"]); // 없으면 생성 필요

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

	LoadAllMonsters(device, m_textures, m_Monsters);

	m_skybox = make_shared<GameObject>(device);
	m_skybox->SetMesh(m_meshes["SKYBOX"]);
	m_skybox->SetTextureIndex(m_textures["SKYBOX"]->GetTextureIndex());
	m_skybox->SetTexture(m_textures["SKYBOX"]);

	m_terrain = make_shared<Terrain>(device);
	m_terrain->SetMesh(m_meshes["TERRAIN"]);
	m_terrain->SetTextureIndex(m_textures["TERRAIN"]->GetTextureIndex());
	m_terrain->SetTexture(m_textures["TERRAIN"]);
	m_terrain->SetScale(XMFLOAT3{ 5.f, 0.25f, 5.f });
	m_terrain->SetPosition(XMFLOAT3{ 0.f, -100.f, 0.f });


	for (auto& obj : m_fbxObjects)
	{
		obj->SetTexture(m_textures["FBX"]);
		obj->SetTextureIndex(m_textures["FBX"]->GetTextureIndex());
		obj->SetUseTexture(true); // UV 기반 텍스처 적용
	}
	
}