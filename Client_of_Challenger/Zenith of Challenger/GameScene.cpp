#include "GameScene.h"
#include "monster.h"
#include "MonsterLoader.h"
#include "OtherPlayer.h"
#include "FBXLoader.h"

void GameScene::BuildObjects(const ComPtr<ID3D12Device>& device,
	const ComPtr<ID3D12GraphicsCommandList>& commandList,
	const ComPtr<ID3D12RootSignature>& rootSignature)
{
	PreRender(commandList);
	m_meshes.clear();
	m_textures.clear();
	m_objects.clear();
	m_fbxMeshes.clear(); // FBX 메쉬 초기화

	BuildShaders(device, commandList, rootSignature);
	BuildMeshes(device, commandList);
	BuildTextures(device, commandList);
	BuildMaterials(device, commandList);

	// FBX 파일 로드
	m_fbxLoader = make_shared<FBXLoader>();
	if (m_fbxLoader->LoadFBXModel("Model/Map/Challenge5.fbx",
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

	// 정점맵 파일 로드
	m_ZenithLoader = make_shared<FBXLoader>();
	if (m_ZenithLoader->LoadFBXModel("Model/Map/ZenithObject3.fbx",
		XMMatrixScaling(1.0f, 1.0f, 1.0f)))
	{
		m_ZenithMeshes = m_ZenithLoader->GetMeshes();
	}

	// FBX 모델을 GameObject로 변환 후 m_fbxObjects에 추가
	for (const auto& ZenithMesh : m_ZenithMeshes)
	{
		auto gameObject = make_shared<GameObject>(device);
		gameObject->SetMesh(ZenithMesh);
		gameObject->SetScale(XMFLOAT3{ 0.1f, 0.1f, 0.1f }); // 원래 크기로 유지
		gameObject->SetPosition(XMFLOAT3{ 0.0f, 0.0f, 0.0f }); // Y축 위치 조정

		m_ZenithObjects.push_back(gameObject);
	}



	BuildObjects(device);
}

void GameScene::MouseEvent(HWND hWnd, FLOAT timeElapsed)
{
	if (GetAsyncKeyState(VK_LBUTTON) & 0x0001)
	{
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(gGameFramework->GetHWND(), &pt);

		HandleMouseClick(pt.x, pt.y);

		bool isFull = gGameFramework->GetIsFullScreen(); // 전체화면 여부

		// 강화 버튼 클릭 범위 분기
		bool isEnhanceClicked = false;

		if (isFull)
		{
			if (pt.x >= 166 && pt.x <= 466 && pt.y >= 563 && pt.y <= 629)
				isEnhanceClicked = true;
		}
		else
		{
			if (pt.x >= 126 && pt.x <= 395 && pt.y >= 347 && pt.y <= 438)
				isEnhanceClicked = true;
		}

		if (isEnhanceClicked)
		{
			CS_Packet_ItemState pkt;
			pkt.type = CS_PACKET_ITEMSTATE;
			pkt.enhanceTry = true;
			pkt.size = sizeof(pkt);
			gGameFramework->GetClientNetwork()->SendPacket(reinterpret_cast<const char*>(&pkt), pkt.size);
		}

        // --- 평타 공격 처리 ---
        if (m_ZenithStartGame && !m_player->isPunching)
        {
            m_AttackCollision = true;

            if (m_job == 1) // 전사
            {
                m_player->SetCurrentAnimation("Slash"); // 또는 다른 힐탱커용 평타 애니메이션
				{
					// 네트워크 패킷 전송
					CS_Packet_Animaition pkt;
					pkt.type = CS_PACKET_ANIMATION;
					pkt.animation = 7;
					pkt.size = sizeof(pkt);
					gGameFramework->GetClientNetwork()->SendPacket(reinterpret_cast<const char*>(&pkt), pkt.size);
				}
            }
            else if (m_job == 2) // 마법사
            {
                FireMagicBall(2, m_player->GetRotationY());
				// 다른 플레이어들한테 내가 스킬 뭐 쓰는지 보내주기
				{
					CS_Packet_AttackEffect pkt;
					pkt.type = CS_PACKET_ATTACKEFFECT;
					pkt.size = sizeof(pkt);
					pkt.skill = 2;
					gGameFramework->GetClientNetwork()->SendPacket(reinterpret_cast<const char*>(&pkt), pkt.size);
				}
            }
            else if (m_job == 3) // 힐탱커
            {
            }

            m_player->isPunching = true;
        }

        
    }
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
	}

	if (GetAsyncKeyState(VK_F2) & 0x0001) {
		m_ShadowMapEnabled = !m_ShadowMapEnabled;
	}

	if (GetAsyncKeyState(VK_F3) & 0x0001) // F3 키 단일 입력
	{
		if (m_currentCameraMode == CameraMode::QuarterView)
		{
			m_currentCameraMode = CameraMode::ThirdPerson;
			m_camera = m_thirdPersonCamera;
			// 마우스 숨기고 중앙에 고정
			ShowCursor(FALSE);

			POINT center = { 720, 320 }; // 해상도 1440x1080 기준
			ClientToScreen(gGameFramework->GetHWND(), &center);
			SetCursorPos(center.x, center.y);

			for (auto& [type, group] : m_monsterGroups)
			{
				for (auto& monster : group)
				{
					if (monster) monster->SetCamera(m_camera);
				}
			}

		}
		else
		{
			m_currentCameraMode = CameraMode::QuarterView;
			m_camera = m_quarterViewCamera;
			ShowCursor(TRUE);

			for (auto& [type, group] : m_monsterGroups)
			{
				for (auto& monster : group)
				{
					if (monster) monster->SetCamera(m_camera);
				}
			}
		}

		m_camera->SetLens(0.25f * XM_PI, gGameFramework->GetAspectRatio(), 0.1f, 1000.f);
		m_player->SetCamera(m_camera); // 카메라 바뀐 후 플레이어에도 재등록
	}
	if (GetAsyncKeyState(VK_F4) & 0x0001) { // F4 키 단일 입력
		m_OutLine = !m_OutLine;
		m_bossDied = true;
	}
	if (GetAsyncKeyState(VK_F6) & 0x0001) // 한 번만
	{
		if (!m_bossMonsters.empty())
		{
			auto& boss = m_bossMonsters[0];
			boss->SetCurrentAnimation("Die");
			if (boss && !boss->IsDissolving())
			{
				boss->StartDissolve(); // 디버그용 디졸브 트리거
			}
		}
	}

	if (GetAsyncKeyState(VK_F7) & 0x0001)
	{
		//XMFLOAT3 pos = m_player->GetPosition();
		////OutputDebugStringFormat(L"[Spawn] Player Pos = (%.2f, %.2f, %.2f)\n", pos.x, pos.y, pos.z);
		//if (m_player) {
		//    SpawnDashWarning(pos, m_player->GetRotationY());
		//}
		if (!m_bossMonsters.empty())
		{
			auto& boss = m_bossMonsters[0];
			//boss->SetCurrentAnimation("Polygonal_Metalon_Purple|Polygonal_Metalon_Purple|Die|Animation Base Layer");
			if (boss)
			{
				XMFLOAT3 pos = boss->GetPosition();
				SpawnDashWarning(pos, boss->GetRotationY());
			}
		}

	}

	if (GetAsyncKeyState(VK_OEM_2) & 0x0001) // '/' 키 (VK_OEM_2)
	{
		if (m_bossMonsters[0])
		{
			float yaw = m_bossMonsters[0]->GetRotationY(); // 라디안 값 기준
			yaw += 90.0f;
			m_bossMonsters[0]->SetRotationY(yaw);          // 회전 적용
		}
	}

	if (GetAsyncKeyState(VK_F8) & 0x0001)
	{
		if (m_bossMonsters[0]) {
			SpawnShockwaveWarning(m_bossMonsters[0]->GetPosition());
		}
	}

	if (GetAsyncKeyState('L') & 0x0001) // F7 키 눌렀을 때 한 번만
	{
		ActivateSwordAuraSkill(0);

		CS_Packet_Animaition pkt;
		pkt.type = CS_PACKET_ANIMATION;
		pkt.animation = 4;
		pkt.size = sizeof(pkt);

		gGameFramework->GetClientNetwork()->SendPacket(reinterpret_cast<const char*>(&pkt), pkt.size);
	}

	if (GetAsyncKeyState(VK_OEM_PLUS) & 0x0001) // = 키
	{
		// 패킷 전송
		CS_Packet_SkipChallenge pkt;
		pkt.type = CS_PACKET_SKIPCHALLENGE;
		pkt.skip = true;
		pkt.size = sizeof(pkt);

		gGameFramework->GetClientNetwork()->SendPacket(reinterpret_cast<const char*>(&pkt), pkt.size);
		ActivateZenithStageMonsters();
	}

	if (GetAsyncKeyState(VK_OEM_MINUS) & 0x0001) // - 키
	{
		CS_Packet_StartZenith pkt;
		pkt.type = CS_PACKET_STARTZENITH;
		pkt.size = sizeof(pkt);

		gGameFramework->GetClientNetwork()->SendPacket(reinterpret_cast<const char*>(&pkt), pkt.size);
		m_ZenithStartGame = true;
		SetCameraToggle();
	}
	if (GetAsyncKeyState(VK_OEM_PERIOD) & 0x0001)
	{
		m_job += 1;

		if (m_job == 4) {
			m_job = 1;
		}
	}

	if (GetAsyncKeyState(VK_LEFT) & 0x8000 && m_uiObjects[1]->m_fillAmount > 0.f)
		m_uiObjects[1]->m_fillAmount -= 0.1;

	if (GetAsyncKeyState(VK_RIGHT) & 0x8000 && m_uiObjects[1]->m_fillAmount < 1.0)
		m_uiObjects[1]->m_fillAmount += 0.1;


	if (GetAsyncKeyState(VK_UP) & 0x8000)
	{
		CS_Packet_DebugGold pkt;
		pkt.type = CS_PACKET_DEBUGGOLD;
		pkt.plusGold = true;
		pkt.size = sizeof(pkt);

		gGameFramework->GetClientNetwork()->SendPacket(reinterpret_cast<const char*>(&pkt), pkt.size);
		//m_goldScore = std::min(m_goldScore + 10, 999); // 최대 999까지
	}

	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	{
		if (m_monsterGroups["FrightFly"][0]->GetCurrentAnimation() == "Idle")
		{
			for (int i = 0; i < m_monsterGroups["FrightFly"].size(); i++)
			{
				m_monsterGroups["FrightFly"][i]->PlayAnimationWithBlend("Attack", 0.2f);
				m_monsterGroups["Flower_Fairy"][i]->PlayAnimationWithBlend("Attack", 0.2f);
				m_monsterGroups["Mushroom_Dark"][i]->PlayAnimationWithBlend("Attack", 0.2f);
				m_monsterGroups["Plant_Dionaea"][i]->PlayAnimationWithBlend("Attack", 0.2f);
				m_monsterGroups["Venus_Blue"][i]->PlayAnimationWithBlend("Attack", 0.2f);
			}
		}
		else
		{
			for (int i = 0; i < m_monsterGroups["FrightFly"].size(); i++)
			{
				m_monsterGroups["FrightFly"][i]->PlayAnimationWithBlend("Idle", 0.2f);
				m_monsterGroups["Flower_Fairy"][i]->PlayAnimationWithBlend("Idle", 0.2f);
				m_monsterGroups["Mushroom_Dark"][i]->PlayAnimationWithBlend("Idle", 0.2f);
				m_monsterGroups["Plant_Dionaea"][i]->PlayAnimationWithBlend("Idle", 0.2f);
				m_monsterGroups["Venus_Blue"][i]->PlayAnimationWithBlend("Idle", 0.2f);

			}
		}
	}

	bool isFPressed = (GetAsyncKeyState('F') & 0x8000) != 0;

	if (!m_player->isPunching && isFPressed && !wasKeyPressedF)
	{
		m_AttackCollision = true;
		//m_player->SetCurrentAnimation("Punch.001");
		//m_player->SetCurrentAnimation("Hook");
		if (m_job == 0)
		{
			m_player->SetCurrentAnimation("Kick");
		}
		else
		{
			m_player->SetCurrentAnimation("Slash"); //Goong
		}


		CS_Packet_Animaition pkt;
		pkt.type = CS_PACKET_ANIMATION;
		pkt.animation = 4;
		pkt.size = sizeof(pkt);
		gGameFramework->GetClientNetwork()->SendPacket(reinterpret_cast<const char*>(&pkt), pkt.size);

		m_player->isPunching = true;

	}
	wasKeyPressedF = isFPressed;


	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
	{
		//m_goldScore = std::max(m_goldScore - 10, 0);   // 최소 0까지
	}

	// 1~6 키를 눌렀을 때 해당 인덱스의 숫자 증가
	for (int i = 0; i < 6; ++i)
	{
		if (GetAsyncKeyState('1' + i) & 0x0001)
		{
			CS_Packet_DebugItem pkt;
			pkt.type = CS_PACKET_DEBUGITEM;
			pkt.item = i + 1;
			pkt.size = sizeof(pkt);

			gGameFramework->GetClientNetwork()->SendPacket(reinterpret_cast<const char*>(&pkt), pkt.size);
			//m_inventoryCounts[i] = std::min(m_inventoryCounts[i] + 1, 9);
		}
	}


	if ((m_ZenithEnabled == true) && GetAsyncKeyState('I') & 0x0001)
	{
		m_showReinforcedWindow = !m_showReinforcedWindow;
		//OutputDebugStringA(m_showReinforcedWindow ? "[UI] Reinforced ON\n" : "[UI] Reinforced OFF\n");
	}


	if (GetAsyncKeyState('H') & 0x0001)     // 정점 스테이지 직업별 스킬 공격
	{
		if (m_job == 1) {      // 너가 전사라면
			// 
		}
		else if (m_job == 2) { // 너가 마법사라면
			FireUltimateBulletRain(2, m_player->GetRotationY());
			// 다른 플레이어들한테 내가 스킬 뭐 쓰는지 보내주기
			{
				CS_Packet_AttackEffect pkt;
				pkt.type = CS_PACKET_ATTACKEFFECT;
				pkt.size = sizeof(pkt);
				pkt.skill = 3;
				gGameFramework->GetClientNetwork()->SendPacket(reinterpret_cast<const char*>(&pkt), pkt.size);
			}
		}
		else if (m_job == 3) { // 너가 힐탱커라면
			SpawnHealingObject(2);
		}
		// 다른 플레이어들한테 내가 스킬 뭐 쓰는지 보내주기
		{
			CS_Packet_Animaition pkt;
			pkt.type = CS_PACKET_ANIMATION;
			pkt.animation = 4;
			pkt.size = sizeof(pkt);

			gGameFramework->GetClientNetwork()->SendPacket(reinterpret_cast<const char*>(&pkt), pkt.size);
		}
	}

	if (GetAsyncKeyState('M') & 0x0001) // 궁극기 키
	{
		FireUltimateBulletRain(2, m_player->GetRotationY()); // 본인 기준
	}

	if (GetAsyncKeyState('C') & 0x0001)
	{
		FireMagicBall(2, m_player->GetRotationY());
		m_skillCooldowns = m_skillMaxCooldowns;
	}

	if (GetAsyncKeyState('P') & 0x8000)
	{
		XMFLOAT3 playerPos = m_player->GetPosition();
		SpawnHealingEffect(playerPos);
	}

}

void GameScene::Update(FLOAT timeElapsed)
{
	if (m_currentCameraMode == CameraMode::ThirdPerson)
	{
		m_camera->Update(timeElapsed);

		// 카메라 방향을 기준으로 플레이어 회전
		XMFLOAT3 camDir = m_camera->GetN();
		camDir.y = 0.f;

		if (!Vector3::IsZero(camDir))
		{
			camDir = Vector3::Normalize(camDir);
			float yaw = atan2f(camDir.x, camDir.z);
			//m_player->SetRotationY(XMConvertToDegrees(yaw) + 180.f);
		}
	}

	//칼에 플레이어 손 행렬 곱해주기
	if (m_job == 1)
	{
		// 칼의 원래 로컬 행렬 (즉, 생성 시 초기 위치 → 플레이어 중심에 있어야 함)
		XMMATRIX swordOffset = XMMatrixTranslation(-3000.0f, 2000.0f, -1000.0f); // 칼이 손에서 약간 오른쪽에 있는 형태로 수정 가능        
		// 현재 애니메이션 클립 기준 본 행렬 계산
		const auto& clip = m_player->m_animationClips.at(m_player->m_currentAnim);
		float time = fmod(m_player->m_animTime, clip.duration);

		auto boneIt = m_player->m_boneNameToIndex.find("mixamorig:RightHand");
		if (boneIt != m_player->m_boneNameToIndex.end())
		{
			int boneIndex = boneIt->second;
			auto boneTransforms = clip.GetBoneTransforms(
				time, m_player->m_boneNameToIndex, m_player->m_boneHierarchy,
				m_player->m_boneOffsets, m_player->m_nodeNameToLocalTransform);

			XMMATRIX boneMat = boneTransforms[boneIndex];
			XMMATRIX playerMat = XMLoadFloat4x4(&m_player->GetWorldMatrix());

			//순서 중요!
			XMMATRIX weaponMat = swordOffset * boneMat * playerMat;
			m_weopons[0]->SetWorldMatrix(weaponMat);
			//m_weopons[0]->m_scale=(XMFLOAT3{ 10.f, 10.f, 10.f }); 

		}
		UpdateSwordAuraSkill(timeElapsed); //전사 스킬 업데이트
	}
	//칼에 다른플레이어 손 행렬 곱해주기
	for(int i=0;i<2;i++)
	{
		if (m_otherPlayerJobs[i] == 1)
		{
			// 칼의 원래 로컬 행렬 (즉, 생성 시 초기 위치 → 플레이어 중심에 있어야 함)
			XMMATRIX swordOffset = XMMatrixTranslation(-3000.0f, 2000.0f, -1000.0f); // 칼이 손에서 약간 오른쪽에 있는 형태로 수정 가능        
			// 현재 애니메이션 클립 기준 본 행렬 계산
			const auto& clip = m_Otherplayer[i]->m_animationClips.at(m_Otherplayer[i]->m_currentAnim);
			float time = fmod(m_Otherplayer[i]->m_animTime, clip.duration);

			auto boneIt = m_Otherplayer[i]->m_boneNameToIndex.find("mixamorig:RightHand");
			if (boneIt != m_Otherplayer[i]->m_boneNameToIndex.end())
			{
				int boneIndex = boneIt->second;
				auto boneTransforms = clip.GetBoneTransforms(
					time, m_Otherplayer[i]->m_boneNameToIndex, m_Otherplayer[i]->m_boneHierarchy,
					m_Otherplayer[i]->m_boneOffsets, m_Otherplayer[i]->m_nodeNameToLocalTransform);

				XMMATRIX boneMat = boneTransforms[boneIndex];
				XMMATRIX playerMat = XMLoadFloat4x4(&m_Otherplayer[i]->GetWorldMatrix());

				//순서 중요!
				XMMATRIX weaponMat = swordOffset * boneMat * playerMat;
				m_weopons[i+1]->SetWorldMatrix(weaponMat); 
				//m_weopons[0]->m_scale=(XMFLOAT3{ 10.f, 10.f, 10.f }); 

			}
			//UpdateSwordAuraSkill(timeElapsed); //전사 스킬 업데이트
		}
	}


	m_player->Update(timeElapsed);
	m_sun->Update(timeElapsed);

	for (auto& object : m_objects)
		object->Update(timeElapsed);

	// 플레이어 위치 가져오기
	if (gGameFramework->GetPlayer())
	{
		const XMFLOAT3& playerPos = gGameFramework->GetPlayer()->GetPosition();
	}

	if (!m_ZenithEnabled) { //도전 스테이지
		// [1] 몬스터 업데이트 (map 기반)
		for (auto& [type, group] : m_monsterGroups)
		{
			for (auto& monster : group)
			{
				monster->Update(timeElapsed);
			}
		}

		// [2] 충돌 테스트
		auto playerBox = m_player->GetBoundingBox();

		if (playerBox.Extents.x == 0.f && playerBox.Extents.y == 0.f && playerBox.Extents.z == 0.f)
			return;

		//도전스테이지 일반몬스터와 충돌처리
		for (auto& [type, group] : m_monsterGroups)
		{
			for (size_t i = 0; i < group.size(); ++i)
			{
				int offset = 0;
				if (type == "Mushroom_Dark")
				{
					offset = 0;
				}
				else if (type == "FrightFly")
				{
					offset = 10;
				}
				else if (type == "Plant_Dionaea")
				{
					offset = 20;
				}
				else if (type == "Venus_Blue")
				{
					offset = 30;
				}
				else if (type == "Flower_Fairy")
				{
					offset = 40;
				}
				auto& monster = group[i];

				if (monster->IsDead()) continue;

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

				if (monster->AttackRange.Intersects(m_player->GetBoundingBox()))
				{
					if (!monster->isAttacking)
					{
						monster->isAttacking = true;
						monster->PlayAnimationWithBlend("Attack", 0.2f);
					}
					float time = monster->m_animTime;
					float duration = monster->m_animationClips.at(monster->m_currentAnim).duration;

					if (monster->m_prevAnimTime > time)
					{
						monster->m_didDamageThisAnim = false;
					}

					const auto& clip = monster->m_animationClips.at(monster->m_currentAnim);


					if (monster->m_animTime > clip.duration / 3 && !monster->m_didDamageThisAnim) 
					{
						//m_uiObjects[1]->m_fillAmount -= 0.01;
						monster->m_didDamageThisAnim = true;
						CS_Packet_Damaged pkt; 
						pkt.type = CS_PACKET_DAMAGED;
						pkt.monsterID = offset + static_cast<int>(i);
						pkt.size = sizeof(pkt);

						gGameFramework->GetClientNetwork()->SendPacket(reinterpret_cast<const char*>(&pkt), pkt.size);
						m_AttackCollision = false;
					}

					monster->m_prevAnimTime = time;
				}

				if (intersectX && intersectY && intersectZ)
				{
					if (monster->IsDead() && !monster->IsParticleSpawned())
					{
						for (int i = 0; i < 100; ++i)
						{
							m_particleManager->SpawnParticle(monster->GetPosition());
						}
						monster->MarkParticleSpawned();
						continue;
					}

					m_player->SetPosition(m_player->m_prevPosition); // 이동 되돌리기
					monster->SetBaseColor(XMFLOAT4(1.f, 0.f, 0.f, 1.f)); // 충돌 시 빨강

					monster->isAttacking = true;
					if (getAttackCollision())
					{
						CS_Packet_MonsterHP pkt;
						pkt.type = CS_PACKET_MONSTERHP;
						pkt.monsterID = offset + static_cast<int>(i);
						pkt.damage = 20;
						pkt.size = sizeof(pkt);
						gGameFramework->GetClientNetwork()->SendPacket(reinterpret_cast<const char*>(&pkt), pkt.size);
						m_AttackCollision = false;
					}

				}
				else if (monster->isAttacking && !monster->AttackRange.Intersects(m_player->GetBoundingBox()))
				{

					monster->PlayAnimationWithBlend("Idle", 0.2f);

					monster->SetBaseColor(XMFLOAT4(1.f, 1.f, 1.f, 1.f)); // 기본 흰색
					monster->isAttacking = false;
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
				m_player->SetPosition(m_player->m_prevPosition);
			}
		}


		int score = m_goldScore;
		for (int i = 2; i >= 0; --i)
		{
			int digit = score % 10;
			score /= 10;

			if (i < m_goldDigits.size())
			{
				auto& digitUI = m_goldDigits[i];

				float u0 = (digit * 100.0f) / 1000.0f;  // 픽셀 기준으로 계산
				float u1 = ((digit + 1) * 100.0f) / 1000.0f;

				digitUI->SetCustomUV(u0, 0.0f, u1, 1.0f);
			}
		}

		if (m_particleManager)
		{
			m_particleManager->Update(timeElapsed);
		}

		// 각 인벤토리 숫자의 UV 갱신
		for (int i = 0; i < 6; ++i)
		{
			int digit = m_inventoryCounts[i];

			float u0 = (digit * 100.0f) / 1000.0f;
			float u1 = ((digit + 1) * 100.0f) / 1000.0f;

			m_inventoryDigits[i]->SetCustomUV(u0, 0.0f, u1, 1.0f);
		}
	}
	else //정점 스테이지
	{
		for (auto& [type, group] : m_BossStageMonsters)
		{
			int idxStart = 0;

			if (type == "Mushroom_Dark")            idxStart = 0;
			else if (type == "FrightFly")           idxStart = 5;
			else if (type == "Plant_Dionaea")       idxStart = 10;
			else if (type == "Venus_Blue")          idxStart = 15;
			else if (type == "Flower_Fairy")        idxStart = 20;
			else if (type == "Metalon")             idxStart = 25;

			int idx = idxStart;

			for (auto& monster : group)
			{
				monster->Update(timeElapsed);
				if (idx < gGameFramework->ZmonstersCoord.size()) {
					monster->SetPosition(XMFLOAT3(gGameFramework->ZmonstersCoord[idx].x, gGameFramework->ZmonstersCoord[idx].y, gGameFramework->ZmonstersCoord[idx].z));
					monster->SetRotationY(gGameFramework->ZmonstersToward[idx]);					
					if (monster->m_playMove)
					{
						monster->PlayAnimationWithBlend("Move", 2.0f);
						monster->m_playMove = false;
					}
					++idx;
				}
			}
		}

		// 보스 몬스터 업데이트
		for (auto& boss : m_bossMonsters)
		{
			if (boss)
			{
				boss->Update(timeElapsed);
				boss->SetPosition(XMFLOAT3(gGameFramework->BossCoord.x, gGameFramework->BossCoord.y, gGameFramework->BossCoord.z));
				boss->SetRotationY(gGameFramework->BossToward);
			}
		}

		//스킬 아이콘
		for (int i = 0; i < m_skillIcons.size(); ++i)
		{
			m_skillCooldowns[i] = std::max(0.f, m_skillCooldowns[i] - timeElapsed);

			bool isReady = (m_skillCooldowns[i] <= 0.f);
			m_skillIcons[i]->SetHovered(isReady); // 내부에서 m_isHovered = 1 또는 0
		}

		EndingSceneUpdate(timeElapsed);

		if(m_OtherJobNum[0] == 0) ChangeJob(0);
		if(m_OtherJobNum[1] == 1) ChangeJob(1);

	}

	m_skybox->SetPosition(m_camera->GetEye());

	//힐탱커 스킬 업데이트
	for (const auto& healing : m_healingObjects)
	{
		healing->Update(timeElapsed);
	}

	//마법사 평타 업데이트
	for (auto it = m_magicBalls.begin(); it != m_magicBalls.end();)
	{
		auto& ball = *it;
		ball->Update(timeElapsed);

		if (ball->IsDead())
		{
			it = m_magicBalls.erase(it); // 수명 끝난 매직볼 제거
		}
		else
		{
			++it;
		}
	}

	for (auto& trail : m_trailObjects)
		trail->Update(timeElapsed);

	m_trailObjects.erase(
		std::remove_if(m_trailObjects.begin(), m_trailObjects.end(),
			[](const std::shared_ptr<GameObject>& obj) { return obj->IsDead(); }),
		m_trailObjects.end()
	);


	//마법사 평타 몬스터 충돌처리
	for (auto& ball : m_magicBalls)
	{
		if (!ball->IsActive()) continue;

		const BoundingBox& ballBox = ball->GetBoundingBox();
		const XMFLOAT3& ballCenter = ballBox.Center;
		XMFLOAT3 ballPos = ball->GetPosition();

		XMFLOAT3 ballCenterWorld = {
			ballPos.x + ballCenter.x,
			ballPos.y + ballCenter.y,
			ballPos.z + ballCenter.z
		};

		const XMFLOAT3& ballExtent = ballBox.Extents;

		// 모든 몬스터 대상 충돌 체크
		for (auto& [type, group] : m_monsterGroups)
		{
			for (auto& monster : group)
			{
				if (!monster || monster->IsDead()) continue;

				const BoundingBox& monBox = monster->GetBoundingBox();
				const XMFLOAT3& monCenter = monBox.Center;
				const XMFLOAT3& monExtent = monBox.Extents;

				XMFLOAT3 monPos = monster->GetPosition();
				XMFLOAT3 monCenterWorld = {
					monPos.x + monCenter.x,
					monPos.y + monCenter.y,
					monPos.z + monCenter.z
				};

				bool intersectX = abs(ballCenterWorld.x - monCenterWorld.x) <= (ballExtent.x + monExtent.x);
				bool intersectY = abs(ballCenterWorld.y - monCenterWorld.y) <= (ballExtent.y + monExtent.y);
				bool intersectZ = abs(ballCenterWorld.z - monCenterWorld.z) <= (ballExtent.z + monExtent.z);

				if (intersectX && intersectY && intersectZ)
				{

					monster->ApplyDamage(1.0f); // 데미지 지정

					SpawnMagicImpactEffect(ballCenterWorld);

					ball->SetActive(false);
					break; // 여러 몬스터에게 다중 충돌 막기
				}
			}
		}
	}

	//마법사 평타 이펙트
	for (auto& effect : m_effects)
	{
		if (effect->IsActive())
			effect->Update(timeElapsed);
	}
	m_effects.erase(std::remove_if(m_effects.begin(), m_effects.end(),
		[](const std::shared_ptr<GameObject>& obj) { return !obj->IsActive(); }),
		m_effects.end());


	for (auto it = m_healingEffects.begin(); it != m_healingEffects.end(); )
	{
		auto& effect = *it;

		effect->Update(timeElapsed, m_camera); // 빌보드 처리 + 위치 유지

		if (effect->GetElapsed() > effect->GetLifetime())
			it = m_healingEffects.erase(it);
		else
			++it;
	}

	//먼지 효과 업데이트
	for (auto it = m_dustEffects.begin(); it != m_dustEffects.end(); )
	{
		auto& dust = *it;
		dust->Update(timeElapsed, m_camera);

		if (dust->IsFinished())
			it = m_dustEffects.erase(it);
		else
			++it;
	}

	for (auto it = m_attackIndicators.begin(); it != m_attackIndicators.end(); )
	{
		(*it)->Update(timeElapsed);
		if ((*it)->IsExpired())
			it = m_attackIndicators.erase(it);
		else
			++it;
	}

	if(!m_bossDied) UpdateGameTimeDigits();

	CheckHealingCollision();

}

void GameScene::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	commandList->SetGraphicsRootSignature(gGameFramework->GetRootSignature().Get());
	commandList->SetGraphicsRootDescriptorTable(RootParameter::ShadowMap, gGameFramework->GetShadowMapSrv());

	XMFLOAT3 lightDir = { -1.0f, -1.0f, -1.0f };
	XMVECTOR lightDirection = XMVector3Normalize(XMLoadFloat3(&lightDir));
	XMFLOAT3 center = { 0.0f, 0.0f, 0.0f }; // 전체 씬 중앙 좌표

	XMVECTOR lightPos = XMVectorScale(lightDirection, -300.0f);
	XMVECTOR lightTarget = XMLoadFloat3(&center);
	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX lightView = XMMatrixLookAtLH(lightPos, lightTarget, lightUp);
	XMMATRIX lightProj = XMMatrixOrthographicLH(1000.0f, 1000.0f, 1.0f, 1000.0f);

	m_camera->UpdateShaderVariable(commandList);
	m_lightSystem->UpdateShaderVariable(commandList);

	if (m_ZenithEnabled) {
		m_terrain->SetShader(m_shaders.at("DETAIL"));
		m_terrain->Render(commandList);
	}

	m_skybox->Render(commandList);

	if (m_ZenithEnabled == false) { //맵 토글키
		if (!m_fbxObjects.empty()) //도전 맵 렌더링
		{
			for (const auto& obj : m_fbxObjects) {
				obj->SetShader(m_shaders.at("FBX"));
				obj->Render(commandList);
			}
		}

		// 몬스터 렌더링 (map 기반으로 수정)
		for (const auto& [type, group] : m_monsterGroups)
		{
			if (type == "Metalon") continue;

			for (const auto& monster : group)
			{
				if (!monster->IsActive()) continue;// 5마리만 렌더링

				monster->SetShader(m_shaders.at("FrightFly"));
				monster->Render(commandList);
			}
		}
	}
	else { //정점 몬스터
		for (const auto& object : m_ZenithObjects) //정점 맵 렌더링
		{
			object->SetShader(m_shaders.at("FBX"));
			object->Render(commandList);
		}

		// 보스 몬스터 출력
		for (const auto& boss : m_bossMonsters)
		{
			if (!boss || !boss->IsActive()) continue;
			boss->SetShader(m_shaders.at("BossShader")); // 셰이더 필요시 따로 지정 가능
			boss->Render(commandList);
		}

		//일반 몬스터 출력
		for (const auto& [type, group] : m_BossStageMonsters)
			for (const auto& monster : group)
			{
				if (!monster->IsActive()) continue;
				monster->SetShader(m_shaders.at("FrightFly")); // 필요 시 타입별 셰이더 적용
				monster->Render(commandList);
			}

		for (const auto& weopon : m_weopons)//캐릭터 무기
		{
			weopon->SetShader(m_shaders.at("FBX"));
			weopon->Render(commandList);
		}

		if (m_ZenithStartGame) {
			//스킬 아이콘
			switch (m_job)
			{
			case 1: // 전사
				m_skillIcons[2]->Render(commandList);
				break;
			case 2: // 마법사
				m_skillIcons[0]->Render(commandList);
				break;
			case 3: // 탱커
				m_skillIcons[1]->Render(commandList);
				break;
			default:
				break;
			}
		}

		for (const auto& Banner : m_uiEndingBanner) {
			if(Banner->IsVisible()) Banner->Render(commandList);
		}

		for (const auto& Press : m_uiPressOn) {
			if(Press->IsVisible()) Press->Render(commandList);
		}

	}

	if (m_isSwordSkillActive)
	{
		for (const auto& aura : m_swordAuraObjects)
		{
			aura->Render(commandList);
		}
	}

	for (auto& trail : m_swordAuraTrailList)
	{
		trail.obj->Render(commandList);
	}

	//캐릭터
	if (m_player)
	{
		m_player->SetShader(m_shaders.at("CHARACTER"));
		m_player->Render(commandList);
	}

	for (auto op : m_Otherplayer)
	{
		if (op) {
			op->SetShader(m_shaders.at("CHARACTER"));
			op->Render(commandList);
		}
	}


	if (!m_uiObjects.empty())
	{
		m_shaders.at("UI")->UpdateShaderVariable(commandList);

		float healthRatio = 1.0f /*m_player->GetCurrentHP() / m_player->GetMaxHP()*/;

		for (size_t i = 0; i < m_uiObjects.size(); ++i)
		{
			// Zenith 게임 시작 상태일 경우, index 1만 렌더
			if (m_ZenithStartGame)
			{
				if (i == 1 && m_uiObjects[i])
					m_uiObjects[i]->Render(commandList);
			}
			else
			{
				if (m_uiObjects[i])
					m_uiObjects[i]->Render(commandList);
			}
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

	if (!m_ZenithStartGame) {
		for (const auto& digit : m_goldDigits)
		{
			digit->Render(commandList); // 숫자 각각 렌더
		}

		for (const auto& digit : m_inventoryDigits)
		{
			digit->Render(commandList);
		}
	}

	//파티클 그려주기
	if (m_particleManager)
	{
		m_particleManager->Render(commandList);
	}

	//장비창 그려주기
	if (m_showReinforcedWindow && m_reinforcedWindowUI)
	{
		m_shaders.at("UI")->UpdateShaderVariable(commandList);
		m_reinforcedWindowUI->Render(commandList);
		m_weaponSlotIcon->Render(commandList);
		m_jobSlotIcon->Render(commandList);
		m_plusIcon->Render(commandList);

		for (const auto& digit : m_forcedDigits)
		{
			digit->Render(commandList);
		}
	}

	for (const auto& healing : m_healingObjects)
	{
		healing->SetShader(m_shaders.at("FBX"));
		healing->Render(commandList);
	}

	for (const auto& ball : m_magicBalls)
	{
		if (!ball->IsActive()) continue;
		ball->SetShader(m_shaders.at("MagicBall"));
		ball->Render(commandList);
	}

	for (const auto& trail : m_trailObjects)
	{
		trail->SetShader(m_shaders.at("MagicBall")); // 또는 별도 트레일 셰이더 사용 가능
		trail->Render(commandList);
	}

	for (const auto& effect : m_effects)
	{
		effect->SetShader(m_shaders.at("Impact"));
		effect->Render(commandList);
	}

	for (const auto& Heffect : m_healingEffects)
	{
		Heffect->SetShader(m_shaders.at("HealingEffect"));
		Heffect->Render(commandList);
	}

	//보스 처치 후 먼지 효과
	for (const auto& dust : m_dustEffects)
	{
		dust->SetShader(m_shaders.at("DustEffect"));
		dust->Render(commandList);
	}

	//보스 몬스터 공격 범위 렌더
	for (auto& warning : m_attackIndicators)
	{
		warning->Render(commandList);
	}

	//
	for (const auto& digit : m_timeDigits) {
		digit->Render(commandList);
	}

	for (const auto& Colon : m_ColonDigit) {
		Colon->Render(commandList);
	}


	// 디버그용 그림자맵 시각화
	if (m_debugShadowShader && m_ShadowMapEnabled)
	{
		m_debugShadowShader->Render(commandList, gGameFramework->GetShadowMapSrv());
	}

}



void GameScene::PreRender(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	ID3D12DescriptorHeap* heaps[] = {
	 gGameFramework->GetDescriptorHeap().Get(), //텍스처 리소스
	};
	commandList->SetDescriptorHeaps(_countof(heaps), heaps);

	if (!m_player && m_monsterGroups.empty()) return;

	// 몬스터 본 매트릭스 업로드
	for (const auto& [type, group] : m_monsterGroups)
	{
		for (const auto& monster : group)
		{
			monster->UpdateBoneMatrices(commandList); // 여기서 본 행렬 계산 및 GPU 업로드
		}
	}

	if (m_player) m_player->UpdateBoneMatrices(commandList);

	for (const auto& [type, group] : m_BossStageMonsters)
	{
		for (const auto& monster : group)
		{
			monster->UpdateBoneMatrices(commandList);
		}
	}


	for (int i = 0; i < 2; i++)
	{
		if (m_Otherplayer[i])
			m_Otherplayer[i]->UpdateBoneMatrices(commandList);
	}

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
	// 몬스터 전용 셰이더 추가
	auto frightFlyShader = make_shared<FrightFlyShader>(device, rootSignature);
	m_shaders.insert({ "FrightFly", frightFlyShader });
	// 충돌처리 와이어 프레임
	auto debugLineShader = make_shared<DebugLineShader>(device, rootSignature);
	m_shaders.insert({ "DebugLineShader", debugLineShader });
	// 그림자 출력용 셰이더
	auto shadowShader = make_shared<ShadowMapShader>(device, rootSignature);
	m_shaders.insert({ "SHADOW", shadowShader });

	m_debugShadowShader = std::make_shared<DebugShadowShader>(device, rootSignature);

	auto HealthbarShader = make_shared<HealthBarShader>(device, rootSignature);
	m_shaders.insert({ "HealthBarShader", HealthbarShader });

	//외곽선 전용 셰이더
	auto outlineShader = make_shared<OutlineShader>(device, rootSignature);
	m_shaders.insert({ "OUTLINE", outlineShader });


	auto ShadowSkinnedshader = make_shared<ShadowSkinnedShader>(device, rootSignature);
	m_shaders.insert({ "ShadowSkinned", ShadowSkinnedshader });


	auto Shadowshader = make_shared<ShadowCharSkinnedShader>(device, rootSignature);
	m_shaders.insert({ "SHADOWCHARSKINNED", Shadowshader });

	auto magicBallShader = make_shared<MagicBallShader>(device, rootSignature);
	m_shaders.insert({ "MagicBall", magicBallShader });

	auto ImpactShader = make_shared<MagicImpactShader>(device, rootSignature);
	m_shaders.insert({ "Impact", ImpactShader });

	auto HealEffectShader = make_shared<HealingEffectShader>(device, rootSignature);
	m_shaders.insert({ "HealingEffect", HealEffectShader });

	auto BossShader = make_shared<BossDissolveShader>(device, rootSignature);
	m_shaders.insert({ "BossShader", BossShader });

	auto dustShader = make_shared<DustEffectShader>(device, rootSignature);
	m_shaders.insert({ "DustEffect", dustShader });

	auto attackShader = make_shared<AttackRangeShader>(device, rootSignature);
	m_shaders.insert({ "AttackRange", attackShader });

	auto attackShader2 = make_shared<ShockwaveRangeShader>(device, rootSignature);
	m_shaders.insert({ "AttackRange2", attackShader2 });

	auto SwordauraShader = make_shared<SwordAuraShader>(device, rootSignature);
	m_shaders.insert({ "SwordAura", SwordauraShader });

	auto RestartShader = make_shared<RestartHintShader>(device, rootSignature);
	m_shaders.insert({ "RestartShader", RestartShader });
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
		TEXT("Model/ZenithTerrain.raw"));
	m_meshes.insert({ "TERRAIN", terrainMesh });
	// Frightfly FBX 메쉬 저장
	auto frightflyLoader = make_shared<FBXLoader>();
	//if (frightflyLoader->LoadFBXModel("Model/Monsters/Frightfly/Frightfly_01.fbx", XMMatrixIdentity()))
	//if (frightflyLoader->LoadFBXModel("Model/Monsters/Frightfly/Polygonal Frightfly 09.fbx", XMMatrixIdentity()))
	//if (frightflyLoader->LoadFBXModel("Model/Monsters/Frightfly/FrightFlyBlend.fbx", XMMatrixIdentity()))
	if (frightflyLoader->LoadFBXModel("Model/Monsters/Frightfly/ExportFrightFlyWithMove.fbx", XMMatrixIdentity()))
	{
		auto meshes = frightflyLoader->GetMeshes();
		if (!meshes.empty())
		{
			m_meshLibrary["FrightFly"] = meshes[0]; // 여러 개면 처리 필요

			// 애니메이션 정보도 캐싱
			const auto& clips = frightflyLoader->GetAnimationClips();
			for (auto& clip : clips)
				m_animClipLibrary["FrightFly"][clip.name] = clip;

			m_boneOffsetLibrary["FrightFly"] = frightflyLoader->GetBoneOffsets();
			m_boneNameMap["FrightFly"] = frightflyLoader->GetBoneNameToIndex();
			m_BoneHierarchy["FrightFly"] = frightflyLoader->GetBoneHierarchy();
			m_staticNodeTransforms["FrightFly"] = frightflyLoader->GetStaticNodeTransforms();
			m_nodeNameToLocalTransform["FrightFly"] = frightflyLoader->GetNodeNameToGlobalTransform();
		}
	}
	// Flower_Fairy FBX 메쉬 저장
	auto flowerFairyLoader = make_shared<FBXLoader>();
	//if (flowerFairyLoader->LoadFBXModel("Model/Monsters/Flower_Fairy/FlowerFairyBlender.fbx", XMMatrixIdentity()))//scale 0.1 
	if (flowerFairyLoader->LoadFBXModel("Model/Monsters/Flower_Fairy/ExportFairyWithMove.fbx", XMMatrixIdentity()))//scale 0.1 
	{
		auto meshes = flowerFairyLoader->GetMeshes();
		if (!meshes.empty())
		{
			m_meshLibrary["Flower_Fairy"] = meshes[0];

			const auto& clips = flowerFairyLoader->GetAnimationClips();
			for (auto& clip : clips)
				m_animClipLibrary["Flower_Fairy"][clip.name] = clip;

			m_boneOffsetLibrary["Flower_Fairy"] = flowerFairyLoader->GetBoneOffsets();
			m_boneNameMap["Flower_Fairy"] = flowerFairyLoader->GetBoneNameToIndex();
			m_BoneHierarchy["Flower_Fairy"] = flowerFairyLoader->GetBoneHierarchy();
			m_staticNodeTransforms["Flower_Fairy"] = flowerFairyLoader->GetStaticNodeTransforms();
			m_nodeNameToLocalTransform["Flower_Fairy"] = flowerFairyLoader->GetNodeNameToGlobalTransform();
		}
		else
		{
			//OutputDebugStringA("[FBXLoader] Flower_Fairy 메쉬 없음\n");
		}
	}
	// Mushroom_Dark FBX 메쉬 저장
	auto mushroomDarkLoader = make_shared<FBXLoader>();
	if (mushroomDarkLoader->LoadFBXModel("Model/Monsters/Mushroom_Dark/ExportMushroomWithMove.fbx", XMMatrixIdentity()))//scale 0.1 
		//if (mushroomDarkLoader->LoadFBXModel("Model/Monsters/Mushroom_Dark/Polygonal Mushroom Dark2.fbx", XMMatrixIdentity()))//scale 0.1 
	{
		auto meshes = mushroomDarkLoader->GetMeshes();
		if (!meshes.empty())
		{
			m_meshLibrary["Mushroom_Dark"] = meshes[0];

			const auto& clips = mushroomDarkLoader->GetAnimationClips();
			for (auto& clip : clips)
				m_animClipLibrary["Mushroom_Dark"][clip.name] = clip;

			m_boneOffsetLibrary["Mushroom_Dark"] = mushroomDarkLoader->GetBoneOffsets();
			m_boneNameMap["Mushroom_Dark"] = mushroomDarkLoader->GetBoneNameToIndex();
			m_BoneHierarchy["Mushroom_Dark"] = mushroomDarkLoader->GetBoneHierarchy();
			m_staticNodeTransforms["Mushroom_Dark"] = mushroomDarkLoader->GetStaticNodeTransforms();
			m_nodeNameToLocalTransform["Mushroom_Dark"] = mushroomDarkLoader->GetNodeNameToGlobalTransform();
		}
		else
		{
			//OutputDebugStringA("[FBXLoader] Mushroom_Dark 메쉬 없음\n");
		}
	}
	// Venus_Blue FBX 메쉬 저장
	auto venusBlueLoader = make_shared<FBXLoader>();
	if (venusBlueLoader->LoadFBXModel("Model/Monsters/Venus_Blue/ExportVenus_BlueWithMove.fbx", XMMatrixIdentity()))//scale 0.1 
	{
		auto meshes = venusBlueLoader->GetMeshes();
		if (!meshes.empty())
		{
			m_meshLibrary["Venus_Blue"] = meshes[0];

			const auto& clips = venusBlueLoader->GetAnimationClips();
			for (auto& clip : clips)
				m_animClipLibrary["Venus_Blue"][clip.name] = clip;

			m_boneOffsetLibrary["Venus_Blue"] = venusBlueLoader->GetBoneOffsets();
			m_boneNameMap["Venus_Blue"] = venusBlueLoader->GetBoneNameToIndex();
			m_BoneHierarchy["Venus_Blue"] = venusBlueLoader->GetBoneHierarchy();
			m_staticNodeTransforms["Venus_Blue"] = venusBlueLoader->GetStaticNodeTransforms();
			m_nodeNameToLocalTransform["Venus_Blue"] = venusBlueLoader->GetNodeNameToGlobalTransform();
		}
		else
		{
			OutputDebugStringA("[FBXLoader] Venus_Blue 메쉬 없음\n");
		}
	}
	// Plant_Dionaea FBX 메쉬 저장
	auto Plant_DionaeaLoader = make_shared<FBXLoader>();
	if (Plant_DionaeaLoader->LoadFBXModel("Model/Monsters/Plant_Dionaea/ExportPlant_DionaeaWithMove.fbx", XMMatrixIdentity()))//scale 0.1     
	{
		auto meshes = Plant_DionaeaLoader->GetMeshes();
		if (!meshes.empty())
		{
			m_meshLibrary["Plant_Dionaea"] = meshes[0];

			const auto& clips = Plant_DionaeaLoader->GetAnimationClips();
			for (auto& clip : clips)
				m_animClipLibrary["Plant_Dionaea"][clip.name] = clip;

			m_boneOffsetLibrary["Plant_Dionaea"] = Plant_DionaeaLoader->GetBoneOffsets();
			m_boneNameMap["Plant_Dionaea"] = Plant_DionaeaLoader->GetBoneNameToIndex();
			m_BoneHierarchy["Plant_Dionaea"] = Plant_DionaeaLoader->GetBoneHierarchy();
			m_staticNodeTransforms["Plant_Dionaea"] = Plant_DionaeaLoader->GetStaticNodeTransforms();
			m_nodeNameToLocalTransform["Plant_Dionaea"] = Plant_DionaeaLoader->GetNodeNameToGlobalTransform();
		}
		else
		{
			OutputDebugStringA("[FBXLoader] Plant_Dionaea 메쉬 없음\n");
		}
	}

	// Metalon FBX 메쉬 저장
	auto Metalon = make_shared<FBXLoader>();
	if (Metalon->LoadFBXModel("Model/Monsters/Metalon/ExportBoss.fbx", XMMatrixIdentity()))//scale 0.1     
	{
		auto meshes = Metalon->GetMeshes();
		if (!meshes.empty())
		{
			m_meshLibrary["Metalon"] = meshes[0];

			const auto& clips = Metalon->GetAnimationClips();
			for (auto& clip : clips)
				m_animClipLibrary["Metalon"][clip.name] = clip;

			m_boneOffsetLibrary["Metalon"] = Metalon->GetBoneOffsets();
			m_boneNameMap["Metalon"] = Metalon->GetBoneNameToIndex();
			m_BoneHierarchy["Metalon"] = Metalon->GetBoneHierarchy();
			m_staticNodeTransforms["Metalon"] = Metalon->GetStaticNodeTransforms();
			m_nodeNameToLocalTransform["Metalon"] = Metalon->GetNodeNameToGlobalTransform();
		}
		else
		{
			OutputDebugStringA("[FBXLoader] Metalon 메쉬 없음\n");
		}
	}


	auto healingLoader = make_shared<FBXLoader>();
	if (healingLoader->LoadFBXModel("Model/Skill/HealingObject.fbx", XMMatrixIdentity()))
	{
		auto meshes = healingLoader->GetMeshes();
		if (!meshes.empty())
		{
			m_meshLibrary["Healing"] = meshes[0];
		}
	}

	auto magicBallLoader = make_shared<FBXLoader>();
	if (magicBallLoader->LoadFBXModel("Model/Skill/MagicBall.fbx", XMMatrixIdentity()))
	{
		auto meshes = magicBallLoader->GetMeshes();
		if (!meshes.empty())
		{
			m_meshLibrary["MagicBall"] = meshes[0];
		}
	}

	auto HealingEffect = make_shared<FBXLoader>();
	if (HealingEffect->LoadFBXModel("Model/Skill/Quad1.fbx", XMMatrixIdentity()))
	{
		auto meshes = HealingEffect->GetMeshes();
		if (!meshes.empty())
		{
			m_meshLibrary["HealingEffect"] = meshes[0];
		}
	}

	auto SwordLoader = make_shared<FBXLoader>();
	if (SwordLoader->LoadFBXModel("Model/Weapon/SwordOnly.fbx", XMMatrixIdentity()))
	{
		auto meshes = SwordLoader->GetMeshes();
		if (!meshes.empty())
		{
			m_meshLibrary["Sword"] = meshes[0];
		}
	}

	auto SwordLoader1 = make_shared<FBXLoader>();
	if (SwordLoader1->LoadFBXModel("Model/Weapon/SwordOnly2.fbx", XMMatrixIdentity())) //Sword2_FBX
	{
		auto meshes = SwordLoader1->GetMeshes();
		if (!meshes.empty())
		{
			m_meshLibrary["Sword1"] = meshes[0];
		}
	}

	auto AttackRange = make_shared<FBXLoader>();
	if (AttackRange->LoadFBXModel("Model/Skill/AttackRange.fbx", XMMatrixIdentity()))
	{
		auto meshes = AttackRange->GetMeshes();
		if (!meshes.empty())
		{
			m_meshLibrary["AttackRange"] = meshes[0];
		}
	}

	m_meshLibrary["SkillIcon"] = CreateScreenQuad(device, commandList, 1.15f, 1.15f, 0.99f);

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
		TEXT("Image/Base_Texture.dds"), RootParameter::Texture); //Terrain_01  Base_Texture
	terrainTexture->LoadTexture(device, commandList,
		TEXT("Image/Detail_Texture_7.dds"), RootParameter::Texture); //Detail_Texture_7
	terrainTexture->CreateShaderVariable(device, true);
	m_textures.insert({ "TERRAIN", terrainTexture });

	auto fbxTexture = make_shared<Texture>(device, commandList,
		TEXT("Image/Map/Base Map.dds"), RootParameter::Texture);
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

	auto GoldTexture = make_shared<Texture>(device, commandList,
		TEXT("Image/InGameUI/Gold.dds"), RootParameter::Texture);
	GoldTexture->CreateShaderVariable(device, true);
	m_textures.insert({ "Gold", GoldTexture });

	auto Gold_ScoreTexture = make_shared<Texture>(device, commandList,
		TEXT("Image/InGameUI/Gold_Score.dds"), RootParameter::Texture);
	Gold_ScoreTexture->CreateShaderVariable(device, true);
	m_textures.insert({ "Gold_Score", Gold_ScoreTexture });


	auto Item_numTexture = make_shared<Texture>(device, commandList,
		TEXT("Image/InGameUI/Item_num.dds"), RootParameter::Texture);
	Item_numTexture->CreateShaderVariable(device, true);
	m_textures.insert({ "Item_num", Item_numTexture });

	auto ColonTexture = make_shared<Texture>(device, commandList,
		TEXT("Image/InGameUI/Colon.dds"), RootParameter::Texture);
	ColonTexture->CreateShaderVariable(device, true);
	m_textures.insert({ "Colon", ColonTexture });

	auto reinforcedTexture = make_shared<Texture>(device, commandList,
		TEXT("Image/InGameUI/reinforced_window1.dds"), RootParameter::Texture);
	reinforcedTexture->CreateShaderVariable(device, true);
	m_textures.insert({ "reinforced", reinforcedTexture });

	auto weaponTexture = make_shared<Texture>(device, commandList,
		TEXT("Image/InGameUI/weapon.dds"), RootParameter::Texture);
	weaponTexture->CreateShaderVariable(device, true);
	m_textures.insert({ "weapon", weaponTexture });

	auto jobTexture = make_shared<Texture>(device, commandList,
		TEXT("Image/InGameUI/job.dds"), RootParameter::Texture);
	jobTexture->CreateShaderVariable(device, true);
	m_textures.insert({ "job", jobTexture });

	auto plusTexture = make_shared<Texture>(device, commandList,
		TEXT("Image/InGameUI/plus.dds"), RootParameter::Texture);
	plusTexture->CreateShaderVariable(device, true);
	m_textures.insert({ "plus", plusTexture });

	auto ZenithTexture = make_shared<Texture>(device, commandList,
		TEXT("Image/Map/ZenithTexture.dds"), RootParameter::Texture);
	ZenithTexture->CreateShaderVariable(device, true);
	m_textures.insert({ "Zenith", ZenithTexture });


	auto MetalonTexture = make_shared<Texture>(device, commandList,
		TEXT("Image/Monsters/Polygonal_Metalon_Purple.dds"), RootParameter::Texture);
	MetalonTexture->CreateShaderVariable(device, true);
	m_textures.insert({ "Metalon", MetalonTexture });

	auto SwordTexture = make_shared<Texture>(device, commandList,
		TEXT("Image/Sword1_Albedo_Silver.dds"), RootParameter::Texture);
	SwordTexture->CreateShaderVariable(device, true);
	m_textures.insert({ "Sword", SwordTexture });

	auto DustTexture = make_shared<Texture>(device, commandList,
		TEXT("Textures/Dust.dds"), RootParameter::Texture);
	DustTexture->CreateShaderVariable(device, true);
	m_textures.insert({ "DUST", DustTexture });

	auto Wizard_SkillTexture = make_shared<Texture>(device, commandList,
		TEXT("Image/InGameUI/Wizard_Skill_Icon.dds"), RootParameter::Texture);
	Wizard_SkillTexture->CreateShaderVariable(device, true);
	m_textures.insert({ "Wizard_Skill_Icon", Wizard_SkillTexture });

	auto Heal_SkillTexture = make_shared<Texture>(device, commandList,
		TEXT("Image/InGameUI/Heal_Skill_Icon.dds"), RootParameter::Texture);
	Heal_SkillTexture->CreateShaderVariable(device, true);
	m_textures.insert({ "Heal_Skill_Icon", Heal_SkillTexture });

	auto Sword_SkillTexture = make_shared<Texture>(device, commandList,
		TEXT("Image/InGameUI/Sword_Skill_Icon.dds"), RootParameter::Texture);
	Sword_SkillTexture->CreateShaderVariable(device, true);
	m_textures.insert({ "Sword_Skill_Icon", Sword_SkillTexture });

	auto Result_ClearTexture = make_shared<Texture>(device, commandList,
		TEXT("Image/InGameUI/Result_Clear.dds"), RootParameter::Texture);
	Result_ClearTexture->CreateShaderVariable(device, true);
	m_textures.insert({ "Result_Clear", Result_ClearTexture });

	auto Result_FailTexture = make_shared<Texture>(device, commandList,
		TEXT("Image/InGameUI/Result_Fail.dds"), RootParameter::Texture);
	Result_FailTexture->CreateShaderVariable(device, true);
	m_textures.insert({ "Result_Fail", Result_FailTexture });

	auto Press_on_restartTexture = make_shared<Texture>(device, commandList,
		TEXT("Image/InGameUI/Press_on_restart.dds"), RootParameter::Texture);
	Press_on_restartTexture->CreateShaderVariable(device, true);
	m_textures.insert({ "Press_on_restart", Press_on_restartTexture });
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
	m_quarterViewCamera = make_shared<QuarterViewCamera>(device);
	m_thirdPersonCamera = make_shared<ThirdPersonCamera>(device);

	m_camera = m_quarterViewCamera;  // 기본 카메라는 쿼터뷰
	m_camera->SetLens(0.25f * XM_PI, gGameFramework->GetAspectRatio(), 0.1f, 1000.f);


	m_lightSystem = make_unique<LightSystem>(device);
	auto sunLight = make_shared<DirectionalLight>();
	m_lightSystem->SetLight(sunLight);

	m_sun = make_unique<Sun>(sunLight);
	m_sun->SetStrength(XMFLOAT3{ 1.3f, 1.3f, 1.3f }); //디렉셔널 라이트 세기 줄이기

	// [2] FBX 로더 생성 및 모델 로드
	m_playerLoader = make_shared<FBXLoader>();
	cout << "캐릭터 로드 중!!!!" << endl;

	if (m_playerLoader->LoadFBXModel("Model/Player/ExportCharacter_fixtalmoheadWeightPaintSubstract.fbx", XMMatrixIdentity()))
		//if (m_playerLoader->LoadFBXModel("Model/Player/TestWithoutSword.fbx", XMMatrixIdentity()))
	{
		auto& meshes = m_playerLoader->GetMeshes();
		if (meshes.empty()) {
			OutputDebugStringA("[ERROR] FBX에서 메시를 찾을 수 없습니다.\n");
			return;
		}

		// [3] Player 객체 생성
		auto player = make_shared<Player>(device);


		player->SetScale(XMFLOAT3{ 0.0005,0.0005,0.0005 }); // 기본값 확정		
		player->SetRotationY(0.f);                  // 정면을 보게 초기화

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
		player->SetBoneHierarchy(m_playerLoader->GetBoneHierarchy());
		player->SetstaticNodeTransforms(m_playerLoader->GetStaticNodeTransforms());
		player->SetNodeNameToGlobalTransform(m_playerLoader->GetNodeNameToGlobalTransform());

		// [7] 텍스처, 머티리얼 설정
		player->SetTexture(m_textures["CHARACTER"]);
		player->SetTextureIndex(m_textures["CHARACTER"]->GetTextureIndex());
		player->SetMaterial(m_materials["CHARACTER"]); // 없으면 생성 필요
		player->SetShader(m_shaders["CHARACTER"]); // 없으면 생성 필요
		player->SetDebugLineShader(m_shaders["DebugLineShader"]);

		// m_player 생성 이후 위치
		BoundingBox playerBox;
		playerBox.Center = XMFLOAT3{ 0.f, 4.5f, 0.f };
		playerBox.Extents = { 1.0f, 4.2f, 1.0f }; // 스케일링된 값
		player->SetPlayerBoundingBox(playerBox);

		BoundingBox playerAttBox;
		playerAttBox.Center = XMFLOAT3{ 0.f, 0.0f, 0.f };
		playerAttBox.Extents = { 2.0f, 4.0f, 2.0f };
		player->SetAttBoundingBox(playerAttBox);

		// [8] 본 행렬 StructuredBuffer용 SRV 생성
		auto [cpuHandle, gpuHandle] = gGameFramework->AllocateDescriptorHeapSlot();
		player->CreateBoneMatrixSRV(device, cpuHandle, gpuHandle);
		OutputDebugStringA((std::string("gGameFramework addr: ") + std::to_string((uintptr_t)gGameFramework.get()) + "\n").c_str());
		// [9] Player 등록 및 GameScene 내부에 저장
		gGameFramework->SetPlayer(player);
		m_player = gGameFramework->GetPlayer();
	}
	else
	{
		OutputDebugStringA("[ERROR] 플레이어 FBX 로드 실패!\n");
	}

	m_player->SetCamera(m_camera);

	if (otherid[0] != -2)
	{
		auto ptr = m_playerLoader->LoadOtherPlayer(device, m_textures, m_shaders);
		if (ptr == nullptr)
			OutputDebugStringA("[ERROR] LoadOtherPlayer 실패\n");
		else
			m_Otherplayer[0] = ptr;
		m_Otherplayer[0]->m_id = otherid[0];
		m_Otherplayer[0]->m_position = otherpos[0];
		m_Otherplayer[0]->SetPosition(m_Otherplayer[0]->m_position);
		m_Otherplayer[0]->SetScale(XMFLOAT3{ 0.0005,0.0005,0.0005 });
	}
	if (otherid[1] != -2)
	{
		m_Otherplayer[1] = m_playerLoader->LoadOtherPlayer(device, m_textures, m_shaders);
		m_Otherplayer[1]->m_id = otherid[1];
		m_Otherplayer[1]->m_position = otherpos[1];
		m_Otherplayer[1]->SetPosition(m_Otherplayer[1]->m_position);
		m_Otherplayer[1]->SetScale(XMFLOAT3{ 0.0005,0.0005,0.0005 });
	}

	array<string, 3> modelPaths = {
	"Model/Player/TestWithoutSword.fbx",//p
	"Model/Player/TestWithoutSword2.fbx",//op1
	"Model/Player/TestWithoutSword3.fbx",//op2
	//"Model/Player/Mage.fbx",
	//"Model/Player/Mage.fbx",
	//"Model/Player/Healer.fbx"
	//"Model/Player/Healer.fbx"
	};

	for (int i = 0; i < 1; ++i) {
		auto loader = make_shared<FBXLoader>();
		if (loader->LoadFBXModel(modelPaths[i], XMMatrixIdentity())) {
			auto player = make_shared<Player>(device);
			for (auto& mesh : loader->GetMeshes())
				player->AddMesh(mesh);

			player->SetScale(XMFLOAT3{ 0.0005, 0.0005, 0.0005 }); // 기본값 확정
			player->SetRotationY(0.f);                  // 정면을 보게 초기화

			player->SetPosition(XMFLOAT3(-580.f, 43.4f, -13.f));

			player->SetAnimationClips(loader->GetAnimationClips());
			player->SetCurrentAnimation("Idle");
			player->SetBoneOffsets(loader->GetBoneOffsets());
			player->SetBoneNameToIndex(loader->GetBoneNameToIndex());
			player->SetBoneHierarchy(loader->GetBoneHierarchy());
			player->SetstaticNodeTransforms(loader->GetStaticNodeTransforms());
			player->SetNodeNameToGlobalTransform(loader->GetNodeNameToGlobalTransform());

			player->SetTexture(m_textures["CHARACTER"]);
			player->SetTextureIndex(m_textures["CHARACTER"]->GetTextureIndex());
			player->SetMaterial(m_materials["CHARACTER"]);
			player->SetShader(m_shaders["CHARACTER"]);
			player->SetDebugLineShader(m_shaders["DebugLineShader"]);
			player->SetCamera(m_camera);

			auto [cpu, gpu] = gGameFramework->AllocateDescriptorHeapSlot();
			player->CreateBoneMatrixSRV(device, cpu, gpu);

			m_jobPlayers[i] = player;
		}
	}

	for (int i = 1; i < 3; ++i) {
		auto loader = make_shared<FBXLoader>();
		if (loader->LoadFBXModel(modelPaths[i], XMMatrixIdentity())) {
			auto Otherplayer = make_shared<OtherPlayer>(device);
			for (auto& mesh : loader->GetMeshes())
				Otherplayer->AddMesh(mesh);

			Otherplayer->SetScale(XMFLOAT3{ 0.0005, 0.0005, 0.0005 }); // 기본값 확정
			Otherplayer->SetRotationY(0.f);                  // 정면을 보게 초기화
			Otherplayer->SetPosition(XMFLOAT3(-580.f, 43.4f, -13.f));
			Otherplayer->SetAnimationClips(loader->GetAnimationClips());
			Otherplayer->SetCurrentAnimation("Idle");
			Otherplayer->SetBoneOffsets(loader->GetBoneOffsets());
			Otherplayer->SetBoneNameToIndex(loader->GetBoneNameToIndex());
			Otherplayer->SetBoneHierarchy(loader->GetBoneHierarchy());
			Otherplayer->SetstaticNodeTransforms(loader->GetStaticNodeTransforms());
			Otherplayer->SetNodeNameToGlobalTransform(loader->GetNodeNameToGlobalTransform());

			Otherplayer->SetTexture(m_textures["CHARACTER"]);
			Otherplayer->SetTextureIndex(m_textures["CHARACTER"]->GetTextureIndex());
			Otherplayer->SetShader(m_shaders["CHARACTER"]);
			Otherplayer->SetDebugLineShader(m_shaders["DebugLineShader"]);

			BoundingBox playerBox;
			playerBox.Center = XMFLOAT3{ 0.f, 4.0f, 0.f };
			playerBox.Extents = { 1.0f, 4.0f, 1.0f };
			Otherplayer->SetBoundingBox(playerBox);

			auto [cpu, gpu] = gGameFramework->AllocateDescriptorHeapSlot(); 
			Otherplayer->CreateBoneMatrixSRV(device, cpu, gpu); 

			m_jobOtherPlayers[i - 1] = Otherplayer;
		}
	}

	auto swordMeshes = m_meshLibrary["Sword"];

	auto swordObject = make_shared<Sword>(device);
	swordObject->SetMesh(swordMeshes);
	swordObject->SetShader(m_shaders["FBX"]);  // FBX 전용 셰이더 사용 
	swordObject->SetTexture(m_textures["Sword"]); // 텍스처는 적절한 걸 할당 
	swordObject->SetTextureIndex(m_textures["Sword"]->GetTextureIndex()); // 텍스처는 적절한 걸 할당 
	swordObject->SetUseTexture(true);
	// 보기 좋게 위치 및 크기 조정 (원한다면)	
	swordObject->SetPosition(XMFLOAT3{ -172.0f, 5.1f, 77.0f });

	m_weopons.push_back(swordObject); // 또는 m_weaponPreviewObject 등으로 따로 저장해도 됨 

	auto swordObject2 = make_shared<Sword>(device);
	swordObject2->SetMesh(swordMeshes);
	swordObject2->SetShader(m_shaders["FBX"]);  // FBX 전용 셰이더 사용 
	swordObject2->SetTexture(m_textures["Sword"]); // 텍스처는 적절한 걸 할당 
	swordObject2->SetTextureIndex(m_textures["Sword"]->GetTextureIndex()); // 텍스처는 적절한 걸 할당 
	swordObject2->SetUseTexture(true);
	// 보기 좋게 위치 및 크기 조정 (원한다면)	
	swordObject2->SetPosition(XMFLOAT3{ -172.0f, 5.1f, 77.0f });

	m_weopons.push_back(swordObject2); // 또는 m_weaponPreviewObject 등으로 따로 저장해도 됨 

	auto swordObject3 = make_shared<Sword>(device);
	swordObject3->SetMesh(swordMeshes);
	swordObject3->SetShader(m_shaders["FBX"]);  // FBX 전용 셰이더 사용 
	swordObject3->SetTexture(m_textures["Sword"]); // 텍스처는 적절한 걸 할당 
	swordObject3->SetTextureIndex(m_textures["Sword"]->GetTextureIndex()); // 텍스처는 적절한 걸 할당 
	swordObject3->SetUseTexture(true);
	// 보기 좋게 위치 및 크기 조정 (원한다면)	
	swordObject3->SetPosition(XMFLOAT3{ -172.0f, 5.1f, 77.0f });

	m_weopons.push_back(swordObject3); // 또는 m_weaponPreviewObject 등으로 따로 저장해도 됨 



	//맵의 오브젝트들 바운딩 박스

	AddCubeCollider({ -212, 0, -211 }, { 21, 15, 20 });
	AddCubeCollider({ -209, 0, -107 }, { 9, 23, 13 });
	AddCubeCollider({ -200, 0, 70 }, { 20, 15, 20 });
	AddCubeCollider({ -105, 5, -1 }, { 30, 30, 28 });
	AddCubeCollider({ -130, 0, -80 }, { 20, 20, 13 });
	AddCubeCollider({ -157, 0, -103 }, { 10, 15, 10 });
	AddCubeCollider({ 31, 0, -136 }, { 20, 16, 18 });
	AddCubeCollider({ 113, 0, -196 }, { 30, 16, 20 });
	AddCubeCollider({ 128, 0, -223 }, { 14, 22, 10 });
	AddCubeCollider({ 137, 10, -86 }, { 15, 10, 60 });
	AddCubeCollider({ -24, 10, -111 }, { 20, 20, 20 });
	AddCubeCollider({ -26, 10, -50 }, { 18, 20, 20 });
	AddCubeCollider({ 204, 10, -117 }, { 15, 10, 15 });
	AddCubeCollider({ 221, 10,-86 }, { 20, 15, 15 });
	AddCubeCollider({ 54, 10, 17 }, { 13, 20, 13 });
	AddCubeCollider({ 64, 10, 43 }, { 13, 20, 13 });
	AddCubeCollider({ -3, 10, 67 }, { 13, 20, 15 });
	AddCubeCollider({ -78, 20, 115 }, { 12, 20, 10 });
	AddCubeCollider({ 107, 10, 152 }, { 12, 15, 12 });
	AddCubeCollider({ 46, 10, 135 }, { 12, 15, 14 });
	AddCubeCollider({ 12, 10, 190 }, { 22, 25, 15 });



	//몬스터 로드
	LoadAllMonsters(
		device,
		m_textures,
		m_shaders,
		m_meshLibrary,
		m_animClipLibrary,
		m_boneOffsetLibrary,
		m_boneNameMap,
		m_BoneHierarchy,
		m_nodeNameToLocalTransform,
		m_monsterGroups,
		m_camera);

	// "Frightfly" 타입 몬스터 배치
	auto& frightflies = m_monsterGroups["FrightFly"];
	for (int i = 0; i < frightflies.size(); ++i)
	{
		frightflies[i]->SetPosition(gGameFramework->CmonstersCoord[i + 10]);
		frightflies[i]->AttackRange.Center = gGameFramework->CmonstersCoord[i + 10];
	}

	// "Flower_Fairy" 타입 몬스터 배치
	auto& fairies = m_monsterGroups["Flower_Fairy"];
	for (int i = 0; i < fairies.size(); ++i)
	{
		fairies[i]->SetPosition(gGameFramework->CmonstersCoord[i + 40]);
		fairies[i]->AttackRange.Center = gGameFramework->CmonstersCoord[i + 40];

	}

	// "Mushroom_Dark" 타입 몬스터 배치
	auto& mushrooms = m_monsterGroups["Mushroom_Dark"];
	for (int i = 0; i < mushrooms.size(); ++i)
	{
		mushrooms[i]->SetPosition(gGameFramework->CmonstersCoord[i]);
		mushrooms[i]->AttackRange.Center = gGameFramework->CmonstersCoord[i];
	}

	// "Venus_Blue" 타입 몬스터 배치
	auto& venusGroup = m_monsterGroups["Venus_Blue"];
	for (int i = 0; i < venusGroup.size(); ++i)
	{
		venusGroup[i]->SetPosition(gGameFramework->CmonstersCoord[i + 30]);
		venusGroup[i]->AttackRange.Center = gGameFramework->CmonstersCoord[i + 30];
	}

	// "Plant_Dionaea" 타입 몬스터 배치
	auto& DionaeaGroup = m_monsterGroups["Plant_Dionaea"];
	for (int i = 0; i < DionaeaGroup.size(); ++i)
	{
		DionaeaGroup[i]->SetPosition(gGameFramework->CmonstersCoord[i + 20]);
		DionaeaGroup[i]->AttackRange.Center = gGameFramework->CmonstersCoord[i + 20];
	}

	// "Metalon" 타입 보스 몬스터 배치
	auto& MetalonGroup = m_monsterGroups["Metalon"];
	for (int i = 0; i < MetalonGroup.size(); ++i)
	{
		MetalonGroup[i]->SetPosition(gGameFramework->ZmonstersCoord[25]);
	}

	int index = 0;

	// FrightFly 5마리
	for (int i = 0; i < 5; ++i, ++index)
	{
		m_monsterGroups["FrightFly"][i]->SetActive(false);
		m_BossStageMonsters["FrightFly"].push_back(m_monsterGroups["FrightFly"][i]);
		m_BossStageMonsters["FrightFly"][i]->SetPosition(gGameFramework->ZmonstersCoord[i + 5]);
	}

	// Flower_Fairy 5마리
	for (int i = 0; i < 5; ++i, ++index)
	{
		m_monsterGroups["Flower_Fairy"][i]->SetActive(false);
		m_BossStageMonsters["Flower_Fairy"].push_back(m_monsterGroups["Flower_Fairy"][i]);
		m_BossStageMonsters["Flower_Fairy"][i]->SetPosition(gGameFramework->ZmonstersCoord[i + 20]);
	}

	// Mushroom_Dark 5마리
	for (int i = 0; i < 5; ++i, ++index)
	{
		m_monsterGroups["Mushroom_Dark"][i]->SetActive(false);
		m_BossStageMonsters["Mushroom_Dark"].push_back(m_monsterGroups["Mushroom_Dark"][i]);
		m_BossStageMonsters["Mushroom_Dark"][i]->SetPosition(gGameFramework->ZmonstersCoord[i]);
	}

	// Venus_Blue 5마리
	for (int i = 0; i < 5; ++i, ++index)
	{
		m_monsterGroups["Venus_Blue"][i]->SetActive(false);
		m_BossStageMonsters["Venus_Blue"].push_back(m_monsterGroups["Venus_Blue"][i]);
		m_BossStageMonsters["Venus_Blue"][i]->SetPosition(gGameFramework->ZmonstersCoord[i + 15]);
	}

	// Plant_Dionaea 5마리
	for (int i = 0; i < 5; ++i, ++index)
	{
		m_monsterGroups["Plant_Dionaea"][i]->SetActive(false);
		m_BossStageMonsters["Plant_Dionaea"].push_back(m_monsterGroups["Plant_Dionaea"][i]);
		m_BossStageMonsters["Plant_Dionaea"][i]->SetPosition(gGameFramework->ZmonstersCoord[i + 10]);
	}

	//스카이박스
	m_skybox = make_shared<GameObject>(device);
	m_skybox->SetMesh(m_meshes["SKYBOX"]);
	m_skybox->SetTextureIndex(m_textures["SKYBOX"]->GetTextureIndex());
	m_skybox->SetTexture(m_textures["SKYBOX"]);
	m_skybox->SetShader(m_shaders["SKYBOX"]);
	m_skybox->SetPosition(m_camera->GetEye());

	//터레인
	m_terrain = make_shared<Terrain>(device);
	m_terrain->SetMesh(m_meshes["TERRAIN"]);
	m_terrain->SetTextureIndex(m_textures["TERRAIN"]->GetTextureIndex());
	m_terrain->SetTexture(m_textures["TERRAIN"]);
	m_terrain->SetShader(m_shaders["DETAIL"]);
	m_terrain->SetScale(XMFLOAT3{ 7.0f, 1.f, 7.0f });
	m_terrain->SetPosition(XMFLOAT3{ 0.f, 29.0f, 0.f });
	m_terrain->SetRotationY(180.0f);

	for (auto& obj : m_fbxObjects)
	{
		obj->SetTexture(m_textures["FBX"]);
		obj->SetTextureIndex(m_textures["FBX"]->GetTextureIndex());
		obj->SetShader(m_shaders["FBX"]);
		obj->SetUseTexture(true); // UV 기반 텍스처 적용
	}

	for (auto& obj : m_ZenithObjects)
	{
		obj->SetTexture(m_textures["Zenith"]);
		obj->SetTextureIndex(m_textures["Zenith"]->GetTextureIndex());
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
    healthBarZeroUI->SetHovered(true);
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
    healthBarUI->SetHovered(true);
    m_uiObjects.push_back(healthBarUI);

	auto Inventory = make_shared<GameObject>(device);

    Inventory->SetTexture(m_textures["Inventory"]);  // 우리가 방금 로드한 텍스처 사용
    Inventory->SetTextureIndex(m_textures["Inventory"]->GetTextureIndex());  // 
    Inventory->SetMesh(CreateScreenQuad(device, gGameFramework->GetCommandList(), 0.5f, 0.5f, 0.98f));
    Inventory->SetPosition(XMFLOAT3(0.8f, -0.55f, 0.97f));
    Inventory->SetUseTexture(true);
    Inventory->SetBaseColor(XMFLOAT4(1, 1, 1, 1));
    Inventory->SetHovered(true);
    m_uiObjects.push_back(Inventory);


	auto Portrait = make_shared<GameObject>(device);

    Portrait->SetTexture(m_textures["Portrait"]);  // 우리가 방금 로드한 텍스처 사용
    Portrait->SetTextureIndex(m_textures["Portrait"]->GetTextureIndex());  // 
    Portrait->SetMesh(CreateScreenQuad(device, gGameFramework->GetCommandList(), 0.25f, 0.25f, 0.98f));
    Portrait->SetPosition(XMFLOAT3(-0.9f, -0.4f, 0.98f));
    Portrait->SetUseTexture(true);
    Portrait->SetBaseColor(XMFLOAT4(1, 1, 1, 1));
    Portrait->SetHovered(true);
    m_uiObjects.push_back(Portrait);

	auto Gold = make_shared<GameObject>(device);

    Gold->SetTexture(m_textures["Gold"]);  // 우리가 방금 로드한 텍스처 사용
    Gold->SetTextureIndex(m_textures["Gold"]->GetTextureIndex());
    Gold->SetMesh(CreateScreenQuad(device, gGameFramework->GetCommandList(), 0.25f, 0.25f, 0.98f));
    Gold->SetPosition(XMFLOAT3(0.75f, 0.7f, 0.98f));
    Gold->SetUseTexture(true);
    Gold->SetBaseColor(XMFLOAT4(1, 1, 1, 1));
    Gold->SetHovered(true);
    m_uiObjects.push_back(Gold);

	// Gold 점수 3자리 숫자 UI 생성
	for (int i = 0; i < 3; ++i)
	{
		auto digitUI = std::make_shared<GameObject>(device);

        digitUI->SetTexture(m_textures["Gold_Score"]); // Gold_Score 텍스처 적용
        digitUI->SetTextureIndex(m_textures["Gold_Score"]->GetTextureIndex());
        digitUI->SetShader(m_shaders["UI"]); // UI용 셰이더 사용
        digitUI->SetUseTexture(true);
        digitUI->SetuseCustomUV(1); // customUV 사용 설정
        digitUI->SetHovered(true);
        // 처음에는 모두 0으로 보여야 하니까
        float u0 = 0.0f;
        float u1 = 1.0f;
        digitUI->SetCustomUV(u0, 0.0f, u1, 1.0f);

		// 처음 메쉬 설정 (0 숫자용 UV 기준)
		digitUI->SetMesh(CreateScreenQuadWithCustomUV(
			device,
			gGameFramework->GetCommandList(),
			0.05f,   // 너비
			0.1f,    // 높이
			0.98f,   // 깊이
			u0, 0.0f, u1, 1.0f // UV 좌표
		));

		// 오른쪽 상단에 배치 (NDC 좌표계 기준)
		digitUI->SetPosition(XMFLOAT3(0.9f + i * 0.06f, 0.7f, 0.98f));
		digitUI->SetScale(XMFLOAT3(1.f, 1.f, 1.f));
		digitUI->SetBaseColor(XMFLOAT4(1, 1, 1, 1));

		m_goldDigits.push_back(digitUI);
	}

	m_particleManager = make_shared<ParticleManager>();
	m_particleManager->Initialize(
		gGameFramework->GetDevice(),
		200, // 최대 200개 파티클 풀
		m_meshes["CUBE"], // 간단한 기본 큐브 메쉬로 (또는 별도 파티클 메쉬 만들 수도 있음)
		m_shaders["HealthBarShader"]
	);

	//장비창 아이템 개수
	for (int i = 0; i < 6; ++i)
	{
		auto digitUI = std::make_shared<GameObject>(device);

        digitUI->SetTexture(m_textures["Item_num"]); // 텍스처 지정
        digitUI->SetTextureIndex(m_textures["Item_num"]->GetTextureIndex());
        digitUI->SetShader(m_shaders["UI"]);
        digitUI->SetUseTexture(true);
        digitUI->SetuseCustomUV(1);
        digitUI->SetHovered(true);
        float u0 = 0.0f;
        float u1 = 1.0f;

		digitUI->SetCustomUV(u0, 0.0f, u1, 1.0f);
		digitUI->SetMesh(CreateScreenQuadWithCustomUV(
			device, gGameFramework->GetCommandList(), 0.03f, 0.06f, 0.98f, u0, 0.0f, u1, 1.0f));

		digitUI->SetPosition(XMFLOAT3(0.7f + (i % 3) * 0.15f, -0.5f - (i / 3) * 0.2f, 0.99f));
		digitUI->SetScale(XMFLOAT3(1.f, 1.f, 1.f));
		digitUI->SetBaseColor(XMFLOAT4(1, 1, 1, 1));

		m_inventoryDigits.push_back(digitUI);
	}

    // 강화창 UI 오브젝트 생성
    m_reinforcedWindowUI = make_shared<GameObject>(device);
    m_reinforcedWindowUI->SetTexture(m_textures["reinforced"]);
    m_reinforcedWindowUI->SetTextureIndex(m_textures["reinforced"]->GetTextureIndex());
    m_reinforcedWindowUI->SetMesh(CreateScreenQuad(device, gGameFramework->GetCommandList(), 0.8f, 1.1f, 0.97f));
    m_reinforcedWindowUI->SetPosition(XMFLOAT3(-0.73f, 0.28f, 0.97f));  // 정중앙
    m_reinforcedWindowUI->SetUseTexture(true);
    m_reinforcedWindowUI->SetBaseColor(XMFLOAT4(1, 1, 1, 1));
    m_reinforcedWindowUI->SetHovered(true);

    //무기 아이콘 슬롯
    m_weaponSlotIcon = make_shared<GameObject>(device);
    m_weaponSlotIcon->SetMesh(CreateScreenQuadWithCustomUV(
        device,
        gGameFramework->GetCommandList(),
        0.14f, 0.13f, 0.97f,
        0.0f, 0.0f, 1.0f, 1.0f  // 초기에는 보이지 않게
    ));
    m_weaponSlotIcon->SetTexture(m_textures.at("weapon"));
    m_weaponSlotIcon->SetPosition({ -0.335f, 0.06f, 0.0f });
    m_weaponSlotIcon->SetuseCustomUV(1);
    m_weaponSlotIcon->SetUseTexture(true);
    m_weaponSlotIcon->SetCustomUV(0.0f, 0.0f, 0.0f, 0.0f);
    m_weaponSlotIcon->SetVisible(true);
    m_weaponSlotIcon->SetBaseColor(XMFLOAT4(1, 1, 1, 1));
    m_weaponSlotIcon->SetShader(m_shaders.at("UI"));
    m_weaponSlotIcon->SetTextureIndex(m_textures["weapon"]->GetTextureIndex());
    m_weaponSlotIcon->SetHovered(true);
    // 직업 아이콘 슬롯
    m_jobSlotIcon = make_shared<GameObject>(device);
    m_jobSlotIcon->SetMesh(CreateScreenQuadWithCustomUV(
        device,
        gGameFramework->GetCommandList(),
        0.14f, 0.13f, 0.97f,
        0.0f, 0.0f, 1.0f, 1.0f
    ));
    m_jobSlotIcon->SetTexture(m_textures.at("job"));
    m_jobSlotIcon->SetPosition({ -0.335f, 0.23f, 0.0f });
    m_jobSlotIcon->SetuseCustomUV(1);
    m_jobSlotIcon->SetUseTexture(true);
    m_jobSlotIcon->SetCustomUV(0.0f, 0.0f, 0.0f, 0.0f);
    m_jobSlotIcon->SetVisible(true);
    m_jobSlotIcon->SetBaseColor(XMFLOAT4(1, 1, 1, 1));
    m_jobSlotIcon->SetShader(m_shaders.at("UI"));
    m_jobSlotIcon->SetTextureIndex(m_textures["job"]->GetTextureIndex());
    m_jobSlotIcon->SetHovered(true);
    //'+'아이콘 이미지
    m_plusIcon = make_shared<GameObject>(device);
    m_plusIcon->SetMesh(CreateScreenQuad(device, gGameFramework->GetCommandList(), 0.1f, 0.1f, 0.97f));
    m_plusIcon->SetTexture(m_textures["plus"]);
    m_plusIcon->SetTextureIndex(m_textures["plus"]->GetTextureIndex());
    m_plusIcon->SetPosition(XMFLOAT3(-0.6f, 0.05f, 0.97f));
    m_plusIcon->SetUseTexture(true);
    m_plusIcon->SetHovered(true);

	auto forcedDigit = std::make_shared<GameObject>(device);

    forcedDigit->SetTexture(m_textures["Gold_Score"]); // Gold_Score 텍스처 적용
    forcedDigit->SetTextureIndex(m_textures["Gold_Score"]->GetTextureIndex());
    forcedDigit->SetShader(m_shaders["UI"]); // UI용 셰이더 사용
    forcedDigit->SetUseTexture(true);
    forcedDigit->SetuseCustomUV(1); // customUV 사용 설정
    forcedDigit->SetHovered(true);
    float u0 = 0.0f;
    float u1 = 0.32f;
    forcedDigit->SetCustomUV(u0, 0.0f, u1, 1.0f);

	// 처음 메쉬 설정 (0 숫자용 UV 기준)
	forcedDigit->SetMesh(CreateScreenQuadWithCustomUV(
		device,
		gGameFramework->GetCommandList(),
		0.05f,   // 너비
		0.1f,    // 높이
		0.99f,   // 깊이
		u0, 0.0f, u1, 1.0f // UV 좌표
	));

	// 오른쪽 상단에 배치 (NDC 좌표계 기준)
	forcedDigit->SetPosition(XMFLOAT3(-0.55f, 0.05f, 0.99f));
	//forcedDigit->SetScale(XMFLOAT3(0.1f, 0.1f, 0.1f));
	forcedDigit->SetBaseColor(XMFLOAT4(1, 1, 1, 1));

	m_forcedDigits.push_back(forcedDigit);

	auto& metalonGroup = m_monsterGroups["Metalon"];
	if (!metalonGroup.empty())
	{
		metalonGroup[0]->SetActive(false);
		m_bossMonsters.push_back(metalonGroup[0]); // 첫 번째 보스만 따로 저장
	}


	XMFLOAT3 startPos = { -1.05f, 0.7f, 0.99f };
	float spacing = 0.07f;
	float colonGap = 0.05f; // 가운데 콜론 간격 추가

    for (int i = 0; i < 4; ++i) // 숫자 4자리만
    {
        auto digitObj = std::make_shared<GameObject>(device);
        digitObj->SetTexture(m_textures["Item_num"]);
        digitObj->SetTextureIndex(m_textures["Item_num"]->GetTextureIndex());
        digitObj->SetShader(m_shaders["UI"]);
        digitObj->SetUseTexture(true);
        digitObj->SetuseCustomUV(1);
        digitObj->SetHovered(true);

		float u0 = 0.0f;
		float u1 = 0.3f;

		digitObj->SetCustomUV(u0, 0.0f, u1, 1.0f);
		digitObj->SetMesh(CreateScreenQuadWithCustomUV(
			device, gGameFramework->GetCommandList(),
			0.03f, 0.06f, 0.98f, u0, 0.0f, u1, 1.0f));
		digitObj->SetScale({ 2.5f, 2.3f, 1.0f });

		// 가운데 콜론 이후 자리는 살짝 더 멀리 배치
		float xOffset = i * spacing;
		if (i >= 2) xOffset += colonGap;

		digitObj->SetPosition({ startPos.x + xOffset, startPos.y, startPos.z });

		m_timeDigits.push_back(digitObj);
	}

	// ':' 따로 출력
	auto ColonDigit = std::make_shared<GameObject>(device);

    ColonDigit->SetTexture(m_textures["Colon"]); // Gold_Score 텍스처 적용
    ColonDigit->SetTextureIndex(m_textures["Colon"]->GetTextureIndex());
    ColonDigit->SetShader(m_shaders["UI"]); // UI용 셰이더 사용
    ColonDigit->SetUseTexture(true);
    ColonDigit->SetuseCustomUV(1); // customUV 사용 설정
    ColonDigit->SetHovered(true);
    u0 = 0.0f;
    u1 = 1.1f;
    ColonDigit->SetCustomUV(u0, 0.0f, u1, 1.0f);

	// 처음 메쉬 설정 (0 숫자용 UV 기준)
	ColonDigit->SetMesh(CreateScreenQuadWithCustomUV(
		device,
		gGameFramework->GetCommandList(),
		0.15f,   // 너비
		0.15f,    // 높이
		0.99f,   // 깊이
		u0, 0.0f, u1, 1.0f // UV 좌표
	));

	// 오른쪽 상단에 배치 (NDC 좌표계 기준)
	ColonDigit->SetPosition(XMFLOAT3(-0.905f, 0.7f, 0.99f));
	ColonDigit->SetScale(XMFLOAT3(1.0f, 0.7f, 1.0f));
	ColonDigit->SetBaseColor(XMFLOAT4(1, 1, 1, 1));

	m_ColonDigit.push_back(ColonDigit);

	auto icon1 = make_shared<GameObject>(device);
	icon1->SetMesh(m_meshLibrary["SkillIcon"]);
	icon1->SetTexture(m_textures["Wizard_Skill_Icon"]); //마법사
	icon1->SetTextureIndex(m_textures["Wizard_Skill_Icon"]->GetTextureIndex());
	icon1->SetShader(m_shaders["UI"]);
	icon1->SetPosition({ 0.9f, -0.65f, 0.99f }); // NDC 좌표 기준
	icon1->SetScale({ 0.25f, 0.25f, 1.0f });

	auto icon2 = make_shared<GameObject>(device);
	icon2->SetMesh(m_meshLibrary["SkillIcon"]);
	icon2->SetTexture(m_textures["Heal_Skill_Icon"]); //힐러
	icon2->SetTextureIndex(m_textures["Heal_Skill_Icon"]->GetTextureIndex());
	icon2->SetShader(m_shaders["UI"]);
	icon2->SetPosition({ 0.9f, -0.65f, 0.99f });
	icon2->SetScale({ 0.25f, 0.25f, 1.0f });

	auto icon3 = make_shared<GameObject>(device);
	icon3->SetMesh(m_meshLibrary["SkillIcon"]);
	icon3->SetTexture(m_textures["Sword_Skill_Icon"]); //전사
	icon3->SetTextureIndex(m_textures["Sword_Skill_Icon"]->GetTextureIndex());
	icon3->SetShader(m_shaders["UI"]);
	icon3->SetPosition({ 0.9f, -0.65f, 0.99f });
	icon3->SetScale({ 0.25f, 0.25f, 1.0f });

	// 저장
	m_skillIcons = { icon1, icon2, icon3 };

	//승리UI
	auto Banner_Clear = make_shared<GameObject>(device);
	Banner_Clear->SetMesh(m_meshLibrary["SkillIcon"]);
	Banner_Clear->SetTexture(m_textures["Result_Clear"]);
	Banner_Clear->SetTextureIndex(m_textures["Result_Clear"]->GetTextureIndex());
	Banner_Clear->SetShader(m_shaders["UI"]);
	Banner_Clear->SetPosition({ 0.69f, -0.22f, 0.98f });
	Banner_Clear->SetScale({ 0.6f, 0.6f, 1.0f });
	Banner_Clear->SetHovered(true);
	Banner_Clear->SetVisible(false);
	m_uiEndingBanner.push_back(Banner_Clear);

	//패배UI
	auto Banner_Fail = make_shared<GameObject>(device);
	Banner_Fail->SetMesh(m_meshLibrary["SkillIcon"]);
	Banner_Fail->SetTexture(m_textures["Result_Fail"]);
	Banner_Fail->SetTextureIndex(m_textures["Result_Fail"]->GetTextureIndex());
	Banner_Fail->SetShader(m_shaders["UI"]);
	Banner_Fail->SetPosition({ 0.69f, -0.22f, 0.98f });
	Banner_Fail->SetScale({ 0.6f, 0.6f, 1.0f });
	Banner_Fail->SetHovered(true);
	Banner_Fail->SetVisible(false);
	m_uiEndingBanner.push_back(Banner_Fail);

	//Press on UI
	auto PressOn = make_shared<GameObject>(device);
	PressOn->SetMesh(m_meshLibrary["SkillIcon"]);
	PressOn->SetTexture(m_textures["Press_on_restart"]);
	PressOn->SetTextureIndex(m_textures["Press_on_restart"]->GetTextureIndex());
	PressOn->SetShader(m_shaders["RestartShader"]);
	PressOn->SetPosition({ 0.0f, 0.6f, 0.99f });
	PressOn->SetScale({ 2.0f, 0.25f, 1.0f });
	PressOn->SetHovered(true);
	PressOn->SetVisible(false);
	m_uiPressOn.push_back(PressOn);


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
void GameScene::RenderShadowPass(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	if (!m_player) return;

	// 플레이어 위치 기준
	XMFLOAT3 playerPos = m_player->GetPosition();  // 반드시 이 함수가 존재해야 함
	XMVECTOR playerPosition = XMLoadFloat3(&playerPos);

	// 빛 방향 설정
	XMFLOAT3 lightDir = { -1.0f, -1.0f, -1.0f };
	XMVECTOR lightDirection = XMVector3Normalize(XMLoadFloat3(&lightDir));

	// lightTarget = 플레이어 위치
	XMVECTOR lightTarget = playerPosition;

	// lightPos = 플레이어 위치에서 빛 방향 반대쪽으로 떨어진 위치
	float lightDistance = 150.0f;
	XMVECTOR lightPos = XMVectorAdd(lightTarget, XMVectorScale(lightDirection, -lightDistance));

	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX lightView = XMMatrixLookAtLH(lightPos, lightTarget, lightUp);
	XMMATRIX lightProj = XMMatrixOrthographicLH(500.0f, 500.0f, 1.0f, 1000.0f);

	// GPU에 Shadow 행렬 업로드 (b4)
	m_camera->UploadShadowMatrix(commandList, lightView, lightProj);
	m_player->SetCamera(m_camera);

	// ===== 기존 그림자 렌더링 흐름 유지 =====
	if (m_ZenithEnabled) {
		m_terrain->SetShader(m_shaders.at("SHADOW"));
		m_terrain->Render(commandList);

		for (auto& obj : m_ZenithObjects)
		{
			obj->SetShader(m_shaders.at("SHADOW"));
			obj->Render(commandList);
		}

	}
	else {
		for (auto& obj : m_fbxObjects)
		{
			if (obj.get() == m_player.get()) continue;
			obj->SetShader(m_shaders.at("SHADOW"));
			obj->Render(commandList);
		}
	}

	if (m_player)
	{
		m_player->SetShader(m_shaders.at("SHADOWCHARSKINNED"));
		m_player->Render(commandList);
	}

	for (auto op : m_Otherplayer)
	{
		if (op) {
			op->SetShader(m_shaders.at("SHADOWCHARSKINNED"));
			op->Render(commandList);
		}
	}

	for (const auto& [type, group] : m_monsterGroups)
	{
		for (const auto& monster : group)
		{
			if (!monster->IsActive()) continue;
			monster->SetShader(m_shaders.at("ShadowSkinned"));
			monster->Render(commandList);
		}
	}

	for (const auto& healing : m_healingObjects)
	{
		healing->SetShader(m_shaders.at("SHADOW"));
		healing->Render(commandList);
	}

	for (const auto& ball : m_magicBalls)
	{
		if (!ball->IsActive()) continue;
		ball->SetShader(m_shaders.at("SHADOW"));
		ball->Render(commandList);
	}

	for (const auto& sword : m_weopons) {
		if (sword) {
			sword->SetShader(m_shaders.at("SHADOW"));
			sword->Render(commandList);
		}
	}
}
void GameScene::HandleMouseClick(int mouseX, int mouseY)
{
	bool isFull = gGameFramework->GetIsFullScreen(); // m_isFullScreen 접근용 함수

	// 좌표 범위 테이블 (무기/직업 순서: 칼, 지팡이, 방패, 전사, 마법사, 탱커)
	struct ClickRegion { int left, top, right, bottom; };
	std::vector<ClickRegion> regions;

	if (isFull)
	{
		regions = {
			{1484, 755, 1587, 860},   // 칼
			{1624, 755, 1719, 860},   // 지팡이
			{1756, 755, 1850, 860},   // 방패
			{1484, 900, 1587, 1000},  // 전사
			{1624, 900, 1719, 1000},  // 마법사
			{1756, 900, 1850, 1000}   // 탱커
		};
	}
	else
	{
		regions = {
			{1102, 527, 1178, 598},   // 칼
			{1207, 527, 1278, 598},   // 지팡이
			{1303, 527, 1373, 598},   // 방패
			{1102, 622, 1180, 688},   // 전사
			{1207, 622, 1278, 688},   // 마법사
			{1303, 622, 1373, 688}    // 탱커
		};
	}

	// 공통 처리
	for (int i = 0; i < 6; ++i)
	{
		if (mouseX >= regions[i].left && mouseX <= regions[i].right &&
			mouseY >= regions[i].top && mouseY <= regions[i].bottom)
		{
			if (m_inventoryCounts[i] > 0)
			{
				CS_Packet_Inventory pkt;
				pkt.type = CS_PACKET_INVENTORY;
				pkt.item = i + 1;
				pkt.size = sizeof(pkt);

				gGameFramework->GetClientNetwork()->SendPacket(reinterpret_cast<const char*>(&pkt), pkt.size);

				if (i < 3 && !m_WeaponOnly)
				{
					m_WeaponOnly = true;
					SetWeaponSlotUV(i); // 칼, 지팡이, 방패
				}
				else if (i >= 3 && !m_JopOnly)
				{
					m_job = i - 2;
					m_JopOnly = true;
					SetJobSlotUV(i - 3); // 전사, 마법사, 탱커
				}
			}
		}
	}
}
void GameScene::SetWeaponSlotUV(int type)
{
	// type: 0 = 칼, 1 = 지팡이, 2 = 방패
	float u0 = (type * 100.0f) / 300.0f;
	float u1 = ((type + 1) * 100.0f) / 300.0f;

	m_weaponSlotIcon->SetCustomUV(u0, 0.f, u1, 1.f);
	m_weaponSlotIcon->SetVisible(true);
}
void GameScene::SetJobSlotUV(int type)
{
	// type: 0 = 전사, 1 = 마법사, 2 = 탱커
	float u0 = (type * 100.0f) / 300.0f;
	float u1 = ((type + 1) * 100.0f) / 300.0f;

	m_jobSlotIcon->SetCustomUV(u0, 0.f, u1, 1.f);
	m_jobSlotIcon->SetVisible(true);

	// 전직 처리: 그냥 포인터 교체
	if (type == 0) {
		m_player = m_jobPlayers[type];
		gGameFramework->SetPlayer(m_player);
		m_player->SetCamera(m_camera);
		m_job = type + 1;
	}
}
void GameScene::UpdateEnhanceDigits()
{
	int level = m_upgradeScore;
	int digitCount = static_cast<int>(m_forcedDigits.size());

	for (int i = digitCount - 1; i >= 0; --i)
	{
		int digit = level % 10;
		level /= 10;

		if (i < m_forcedDigits.size())
		{
			auto& digitUI = m_forcedDigits[i];
			float digitWidth = 0.1f; // 숫자 하나당 100픽셀 기준, 전체 1000픽셀
			float u0 = digit * digitWidth;
			float u1 = (float)(digit + 3.0f) * digitWidth;

			digitUI->SetCustomUV(u0, 0.0f, u1, 1.0f);
		}
	}
}
void GameScene::SpawnHealingObject(int num)
{
	auto device = gGameFramework->GetDevice();
	auto healing = make_shared<HealingObject>(device);

	// 메시 및 셰이더 설정
	healing->SetMesh(m_meshLibrary["Healing"]);
	healing->SetShader(m_shaders["FBX"]);

	// 밝은 녹색 색상 설정
	healing->SetBaseColor(XMFLOAT4(0.5f, 1.0f, 0.5f, 1.0f));
	healing->SetUseTexture(false);

	// 위치 설정: 플레이어 근처 예시
	if (num == 2) { // 본인
		auto playerPos = m_player->GetPosition();
		playerPos.y += 10.f; // 지면 위로 살짝 띄움
		healing->SetPosition(playerPos);
		healing->m_ownerJob = m_job; // 내 직업 저장

    }
    else {  // 다른 플레이어
        auto playerPos = otherpos[num];
        playerPos.y += 10.f; // 지면 위로 살짝 띄움
        healing->SetPosition(playerPos);
        healing->m_ownerJob = 3;
    }

	// 크기/회전 조정
	healing->SetScale(XMFLOAT3(0.05f, 0.05f, 0.05f));
	healing->SetRotationY(0.0f);

	BoundingBox healingBox;
	healingBox.Center = XMFLOAT3{ 0.f, 0.0f, 0.f };
	healingBox.Extents = { 3.0f, 10.0f, 3.0f }; // 스케일링된 값
	healing->SetBoundingBox(healingBox);

	m_healingObjects.push_back(healing);
}
void GameScene::FireMagicBall(int num, float angle)
{
	if (m_meshLibrary.find("MagicBall") == m_meshLibrary.end()) return;

	const int numShots = 5;
	const float maxAngleOffset = XMConvertToRadians(3.0f); // 최대 ±3도 퍼짐
	const float speed = 50.0f;

	// 발사 기준 위치
	XMFLOAT3 playerPos;
	XMFLOAT3 forward;

	if (num == 2) // 본인
	{
		playerPos = m_player->GetPosition();
		forward = Vector3::Normalize(m_player->GetForward());
	}
	else // 다른 플레이어
	{
		playerPos = otherpos[num];
		forward = { 0.f, 0.f, 1.f }; // 다른 플레이어가 바라보는 방향 정보 필요
	}

	playerPos.y += 8.0f; // 발사 높이

	XMVECTOR forwardVec = XMLoadFloat3(&forward);

	for (int i = 0; i < numShots; ++i)
	{
		auto ball = make_shared<MagicBall>(m_device);
		ball->SetMesh(m_meshLibrary["MagicBall"]);
		ball->SetShader(m_shaders["MagicBall"]);
		ball->SetPosition(playerPos);

		float angleOffset = Random::Range(-maxAngleOffset, maxAngleOffset);
		XMMATRIX rot = XMMatrixRotationY(angleOffset);
		XMVECTOR shotDir = XMVector3TransformNormal(forwardVec, rot);

		XMFLOAT3 dir;
		XMStoreFloat3(&dir, shotDir);
		dir = Vector3::Normalize(dir);
		ball->SetDirection(dir);
		ball->SetSpeed(speed);
		ball->SetLifetime(3.0f);

		float offsetX = Random::Range(0.0f, XM_2PI);
		float offsetY = Random::Range(0.0f, XM_2PI);
		ball->SetWaveOffsets(offsetX, offsetY);

		float freqX = Random::Range(5.0f, 8.0f);
		float freqY = Random::Range(6.0f, 9.0f);
		float freqZ = Random::Range(4.0f, 7.0f);

		float ampX = Random::Range(0.2f, 0.4f);
		float ampY = Random::Range(0.15f, 0.3f);
		float ampZ = Random::Range(0.2f, 0.35f);

		ball->SetScaleAnimation(freqX, ampX, freqY, ampY, freqZ, ampZ);

		m_magicBalls.push_back(ball);
	}
}
void GameScene::AddTrailObject(const shared_ptr<GameObject>& obj)
{
	m_trailObjects.push_back(obj);
}
void GameScene::SpawnMagicImpactEffect(const XMFLOAT3& pos)
{
	auto effect = make_shared<MagicImpactEffect>(m_device);

	effect->SetMesh(m_meshLibrary["MagicBall"]);  // 임팩트용 메시
	effect->SetShader(m_shaders["Impact"]);    // ImpactShader.hlsl로 만든 셰이더
	effect->SetLifetime(0.5f);
	effect->SetPosition(pos);

	m_effects.push_back(effect);
}
void GameScene::SpawnHealingEffect(const XMFLOAT3& playerPos)
{
	for (int i = 0; i < 10; ++i) {

		auto effect = make_shared<HealingEffectObject>(m_device);
		effect->SetMesh(m_meshLibrary["HealingEffect"]);
		effect->SetShader(m_shaders["HealingEffect"]);
		effect->SetLifetime(1.0f);
		effect->SetFollowTarget(m_player);

		effect->SetPosition(XMFLOAT3(playerPos.x, playerPos.y + 10.f, playerPos.z));
		effect->InitializeParticles();

		m_healingEffects.push_back(effect);
	}
}
void GameScene::ActivateZenithStageMonsters()
{
	for (auto& [type, group] : m_BossStageMonsters)
	{
		for (auto& monster : group)
		{
			monster->SetActive(true);
			monster->PlayAnimationWithBlend("Move", 0.2f);
		}
	}

	for (auto& boss : m_bossMonsters)
	{
		boss->SetActive(true);
	}
}
void GameScene::CheckHealingCollision()
{
	const BoundingBox& playerBox = m_player->GetBoundingBox();
	const XMFLOAT3& playerCenter = playerBox.Center;
	const XMFLOAT3& playerExtent = playerBox.Extents;

	XMFLOAT3 playerCenterWorld = playerCenter;

	for (auto it = m_healingObjects.begin(); it != m_healingObjects.end(); )
	{
		const auto& healing = *it;
		const BoundingBox& healBox = healing->GetBoundingBox();
		const XMFLOAT3& healCenter = healBox.Center;
		const XMFLOAT3& healExtent = healBox.Extents;
		XMFLOAT3 healPos = healing->GetPosition();

		XMFLOAT3 healCenterWorld = {
			healPos.x + healCenter.x,
			healPos.y + healCenter.y,
			healPos.z + healCenter.z
		};

		bool intersectX = abs(playerCenterWorld.x - healCenterWorld.x) <= (playerExtent.x + healExtent.x);
		bool intersectY = abs(playerCenterWorld.y - healCenterWorld.y) <= (playerExtent.y + healExtent.y);
		bool intersectZ = abs(playerCenterWorld.z - healCenterWorld.z) <= (playerExtent.z + healExtent.z);

		bool isSelfCollision = intersectX && intersectY && intersectZ;

		// === [2] 타 플레이어 충돌 검사 ===
		for (int i = 0; i < 2; ++i)
		{
			if (!m_Otherplayer[i]) continue;

			const BoundingBox& otherBox = m_Otherplayer[i]->GetBoundingBox();
			const XMFLOAT3& otherCenter = otherBox.Center;
			const XMFLOAT3& otherExtent = otherBox.Extents;
			XMFLOAT3 otherPos = m_Otherplayer[i]->GetPosition();

			XMFLOAT3 otherCenterWorld = otherCenter;

			bool oX = abs(otherCenterWorld.x - healCenterWorld.x) <= (otherExtent.x + healExtent.x);
			bool oY = abs(otherCenterWorld.y - healCenterWorld.y) <= (otherExtent.y + healExtent.y);
			bool oZ = abs(otherCenterWorld.z - healCenterWorld.z) <= (otherExtent.z + healExtent.z);

			if (oX && oY && oZ)
			{
				// 타 플레이어도 힐탱커고 힐팩도 힐탱커가 만든 거면 무시
				if (m_otherPlayerJobs[i] == 3 && healing->m_ownerJob == 3)
				{
					++it;
					goto next_heal;
				}

				// 내가 힐탱커일 경우 → 힐팩은 못 먹어도 이펙트는 보여줌
				if (m_job == 3 && healing->m_ownerJob == 3)
				{
					SpawnHealingEffect(otherPos);
				}

				it = m_healingObjects.erase(it);
				goto next_heal;
			}
		}

		// === [3] 본인 충돌 ===
		if (isSelfCollision)
		{
			if (m_job == 3 && healing->m_ownerJob == 3)
			{
				++it;
				continue;
			}

			SpawnHealingEffect(m_player->GetPosition());
			it = m_healingObjects.erase(it);
		}
		else
		{
			++it;
		}

	next_heal:;
	}
}
void GameScene::FireUltimateBulletRain(int num, float yaw)
{
	if (m_meshLibrary.find("MagicBall") == m_meshLibrary.end()) return;

	const int numShots = 10;                  // 전체 발사 수
	const float angleRange = XMConvertToRadians(30.0f); // 부채꼴 각도 (±15도)
	const float shotInterval = angleRange / (numShots - 1); // 탄 간격
	const float speed = 80.0f;

	// 발사 기준 위치
	XMFLOAT3 casterPos;
	XMFLOAT3 forward;

	if (num == 2) // 본인
	{
		casterPos = m_player->GetPosition();
		forward = Vector3::Normalize(m_player->GetForward());
	}
	else // 다른 플레이어
	{
		casterPos = otherpos[num];
		forward = { 0.f, 0.f, 1.f }; // 고정 방향 (or 다른 플레이어 회전값 기반으로 확장 가능)
	}

	casterPos.y += 5.0f;

	XMVECTOR forwardVec = XMLoadFloat3(&forward);

	for (int i = 0; i < numShots; ++i)
	{
		float angleOffset = -angleRange * 0.5f + shotInterval * i;
		XMMATRIX rot = XMMatrixRotationY(angleOffset);
		XMVECTOR shotDir = XMVector3TransformNormal(forwardVec, rot);

		XMFLOAT3 dir;
		XMStoreFloat3(&dir, shotDir);
		dir = Vector3::Normalize(dir);

		auto ball = make_shared<MagicBall>(m_device);
		ball->SetMesh(m_meshLibrary["MagicBall"]);
		ball->SetShader(m_shaders["MagicBall"]);
		ball->SetPosition(casterPos);
		ball->SetDirection(dir);
		ball->SetSpeed(speed);
		ball->SetLifetime(1.5f);

		ball->SetBallType(MagicBallType::Ultimate);

		ball->SetWaveOffsets(0.f, 0.f);
		ball->SetScaleAnimation(0.f, 0.f, 0.f, 0.f, 0.f, 0.f); // scale pulse 제거

		m_magicBalls.push_back(ball);
	}
}
void GameScene::ActivateSwordAuraSkill(int num) //0 = 타 클라1 / 1 = 타 클라2 / 2 = 본인
{
	if (m_isSwordSkillActive) return; // 중복 발동 방지

	auto aura = make_shared<SwordAuraObject>(gGameFramework->GetDevice());
	aura->SetMesh(m_meshLibrary["Sword1"]); // 전사 검 메시 공유
	aura->SetShader(m_shaders.at("SwordAura"));
	aura->SetVisible(true);
	aura->SetScale(XMFLOAT3(5.f, 5.f, 5.f));
	m_swordAuraObjects.push_back(aura);
	m_isSwordSkillActive = true;
	m_swordSkillDuration = 0.0f;
	m_SwordNum = num;
}
void GameScene::UpdateSwordAuraSkill(float timeElapsed)
{
	if (!m_isSwordSkillActive) return;

	m_swordSkillDuration += timeElapsed;

	if (m_swordSkillDuration >= MAX_SWORD_SKILL_DURATION)
	{
		m_isSwordSkillActive = false;
		m_swordAuraObjects.clear();
		m_swordAuraTrailList.clear();
		m_SwordNum = 99;
		return;
	}

	// 오라 위치 동기화
	for (auto& aura : m_swordAuraObjects)
	{
		aura->Update(timeElapsed);

		if (!m_weopons.empty() && m_SwordNum != 99)
		{
			XMMATRIX weaponMatrix = XMLoadFloat4x4(&m_weopons[m_SwordNum]->GetWorldMatrix());
			aura->SetWorldMatrix(weaponMatrix);
		}
	}

	// --- 트레일 생성 ---
	m_trailTimer += timeElapsed;
	if (m_trailTimer >= TRAIL_SPAWN_INTERVAL)
	{
		m_trailTimer = 0.0f;

		if (!m_weopons.empty() && m_SwordNum != 99)
		{
			XMFLOAT4X4 mat = m_weopons[m_SwordNum]->GetWorldMatrix();

			auto trail = std::make_shared<SwordAuraObject>(m_device);
			trail->SetMesh(m_meshLibrary["Sword1"]);
			trail->SetShader(m_shaders["SwordAura"]);
			trail->SetWorldMatrix(XMLoadFloat4x4(&mat));
			trail->SetUseTexture(false);
			trail->SetBaseColor(XMFLOAT4(0.2f, 1.f, 1.f, 1.0f)); // 밝은 시안색
			trail->SetVisible(true);

			m_swordAuraTrailList.push_back({ trail, 0.0f });
		}
	}

	// --- 트레일 잔상 페이드 및 제거 ---
	for (auto it = m_swordAuraTrailList.begin(); it != m_swordAuraTrailList.end(); )
	{
		it->life += timeElapsed;
		float alpha = 1.0f - (it->life / TRAIL_LIFETIME);

		if (alpha <= 0.0f)
		{
			it = m_swordAuraTrailList.erase(it);
		}
		else
		{
			XMFLOAT4 color = it->obj->GetBaseColor();
			color.w = alpha;
			it->obj->SetBaseColor(color);
			++it;
		}
	}
}
void GameScene::SpawnDustEffect(const XMFLOAT3& position)
{
	for (int i = 0; i < 30; ++i) {
		auto dust = make_shared<DissolveDustEffectObject>(m_device);
		dust->SetMesh(m_meshLibrary["HealingEffect"]);
		dust->SetShader(m_shaders["DustEffect"]); // DustEffect → HealingEffect 방식 셰이더 사용

		dust->Spawn(position, 1); // 중심 위치에서 30개의 먼지 파티클 생성
		m_dustEffects.push_back(dust);
	}
}
void GameScene::SpawnDashWarning(const XMFLOAT3& pos, float yaw)
{
	auto obj = std::make_shared<AttackRangeIndicator>(m_device);

	obj->SetMesh(m_meshLibrary["AttackRange"]);
	obj->SetScale({ 0.5f, 1.0f, 2.0f }); // 크기

	obj->SetRotationY(yaw); // 회전 먼저 적용

	// 회전된 forward(Z+) 벡터 구하기
	XMMATRIX rot = XMMatrixRotationY(XMConvertToRadians(yaw));
	XMVECTOR forward = XMVector3TransformNormal(
		XMVectorSet(0, 0, 1, 0), // 기본 forward = Z+
		rot
	);

	float distance = -100.0f; // 보스와의 거리
	XMVECTOR posVec = XMLoadFloat3(&pos);
	XMVECTOR offset = XMVectorScale(forward, distance);
	XMVECTOR finalPos = XMVectorAdd(posVec, offset);

	XMFLOAT3 spawnPos;
	XMStoreFloat3(&spawnPos, finalPos);
	spawnPos.y += 1.1f;

	obj->SetPosition(spawnPos); // 위치 반영
	obj->SetFillAmount(0.0f);
	obj->SetShader(m_shaders["AttackRange"]);
	obj->SetLifetime(2.0f);
	m_attackIndicators.push_back(obj);
}
void GameScene::SpawnShockwaveWarning(const XMFLOAT3& pos)
{
	auto obj = std::make_shared<AttackRangeIndicator>(m_device);

	obj->SetMesh(m_meshLibrary["AttackRange"]); // 정사각형 메시

	obj->SetScale({ 2.0f, 1.0f, 2.0f }); // 반지름 3m 원형 효과 (수동 조절)

	XMFLOAT3 adjustedPos = pos;
	adjustedPos.y += 1.1f;
	obj->SetPosition(adjustedPos);

	obj->SetFillAmount(0.0f);
	obj->SetShader(m_shaders["AttackRange2"]);
	obj->SetLifetime(2.0f); // 채워질 시간

	m_attackIndicators.push_back(obj);
}
void GameScene::UpdateGameTimeDigits()
{
	int totalSeconds = static_cast<int>(gGameFramework->GetTotalTime());
	int minutes = totalSeconds / 60;
	int seconds = totalSeconds % 60;

	char buffer[5];
	sprintf_s(buffer, "%02d%02d", minutes, seconds); // "0945" 형식

	for (int i = 0; i < 4; ++i)
	{
		if (!isdigit(buffer[i])) continue;

		int digit = buffer[i] - '0';

		//숫자 하나당 UV 폭 0.3을 사용하는 너의 텍스처 기준
		float u0 = digit * 0.1f;
		float u1 = u0 + 0.31f;

		if (i < m_timeDigits.size())
		{
			m_timeDigits[i]->SetCustomUV(u0, 0.0f, u1, 1.0f);
		}
	}
}
void GameScene::EndingSceneUpdate(float timeElapsed)
{
	if (m_bossDied && !m_showEndingSequence)
	{
		m_showEndingSequence = true;
		m_endingTimer = 0.f;

		m_uiObjects[1]->SetVisible(false);

		for (int i = 0; i < 3; ++i) {
			m_skillIcons[i]->SetVisible(false);
		}
		// 플레이어 위치 → 도착 목표
		m_player->SetPosition({ 570.f, 44.f, 6.5f }); // 예시 위치

		// 시간 UI 이동 시작
		m_moveTimeUI = true;
		m_timeUIMoveTimer = 0.f;


		// [1] 콜론 이동 거리 계산
		m_colonStartPos = m_ColonDigit[0]->GetPosition();
		m_colonTargetPos = { 0.7f, -0.4f, 0.99f }; // 원하는 최종 위치

		XMFLOAT3 moveOffset = {
			m_colonTargetPos.x - m_colonStartPos.x,
			m_colonTargetPos.y - m_colonStartPos.y,
			m_colonTargetPos.z - m_colonStartPos.z
		};

		// [2] 숫자들: 시작 위치 저장 및 동일한 오프셋 적용
		m_timeDigitStartPos.clear();
		m_timeDigitTargetPos.clear();

		for (auto& digit : m_timeDigits)
		{
			XMFLOAT3 cur = digit->GetPosition();
			m_timeDigitStartPos.push_back(cur);

			XMFLOAT3 target = {
				cur.x + moveOffset.x,
				cur.y + moveOffset.y,
				cur.z + moveOffset.z
			};
			m_timeDigitTargetPos.push_back(target);
		}
	}

	if (m_showEndingSequence)
	{
		m_endingTimer += timeElapsed;

		XMFLOAT3 playerPos = m_player->GetPosition();
		m_player->SetRotationY(225.f);
		XMFLOAT3 forward = m_player->GetForward();

		XMFLOAT3 camPos = {
			playerPos.x + forward.x * 20.f,
			playerPos.y + 12.f,
			playerPos.z + forward.z * 20.f
		};

		XMFLOAT3 lookAt = {
			playerPos.x,
			playerPos.y + 5.f,
			playerPos.z
		};

		// 승리 연출용 카메라 고정
		m_camera->SetPosition(camPos);
		m_camera->SetLookAt(lookAt);

		// UI 띄우기
		if (m_endingTimer >= MAX_ENDING_TIME)
		{
			m_uiEndingBanner[0]->SetVisible(true); // 승리 UI 표시
			m_uiPressOn[0]->SetVisible(true);
			//m_uiEndingBanner[1]->SetVisible(true); // 패배 UI 표시
		}
	}

	if (m_moveTimeUI)
	{
		m_timeUIMoveTimer += timeElapsed;
		float t = std::clamp(m_timeUIMoveTimer / MAX_TIMEUI_MOVE_DURATION, 0.0f, 1.0f);

		for (int i = 0; i < m_timeDigits.size(); ++i)
		{
			XMFLOAT3 start = m_timeDigitStartPos[i];
			XMFLOAT3 end = m_timeDigitTargetPos[i];

			XMFLOAT3 lerpPos = {
				start.x + (end.x - start.x) * t,
				start.y + (end.y - start.y) * t,
				0.99f,
			};
			m_timeDigits[i]->SetPosition(lerpPos);
		}

		// 콜론도 이동
		XMFLOAT3 colonPos = {
			m_colonStartPos.x + (m_colonTargetPos.x - m_colonStartPos.x) * t,
			m_colonStartPos.y + (m_colonTargetPos.y - m_colonStartPos.y) * t,
			0.99f,
		};
		m_ColonDigit[0]->SetPosition(colonPos);

		if (t >= 1.0f) m_moveTimeUI = false;
	}



}
void GameScene::SetCameraToggle()
{
	if (m_currentCameraMode == CameraMode::QuarterView)
	{
		m_currentCameraMode = CameraMode::ThirdPerson;
		m_camera = m_thirdPersonCamera;
		// 마우스 숨기고 중앙에 고정
		ShowCursor(FALSE);

		POINT center = { 720, 320 }; // 해상도 1440x1080 기준
		ClientToScreen(gGameFramework->GetHWND(), &center);
		SetCursorPos(center.x, center.y);

		for (auto& [type, group] : m_monsterGroups)
		{
			for (auto& monster : group)
			{
				if (monster) monster->SetCamera(m_camera);
			}
		}

	}
	else
	{
		m_currentCameraMode = CameraMode::QuarterView;
		m_camera = m_quarterViewCamera;
		ShowCursor(TRUE);

		for (auto& [type, group] : m_monsterGroups)
		{
			for (auto& monster : group)
			{
				if (monster) monster->SetCamera(m_camera);
			}
		}
	}

	m_camera->SetLens(0.25f * XM_PI, gGameFramework->GetAspectRatio(), 0.1f, 1000.f);
	m_player->SetCamera(m_camera); // 카메라 바뀐 후 플레이어에도 재등록
}
void GameScene::ChangeJob(int index)
{
	gGameFramework->WaitForGpuComplete();
	//m_jobOtherPlayers[index]->SetPosition(otherpos[index]); 
	m_jobOtherPlayers[index]->oldPos = otherpos[index];  
	m_otherPlayerJobs[index] = 1;
	m_Otherplayer[index] = m_jobOtherPlayers[index]; //전사 player
	m_Otherplayer[index]->m_id = otherid[index];
	m_Otherplayer[index]->SetPosition(otherpos[index]);
	//auto [cpu, gpu] = gGameFramework->AllocateDescriptorHeapSlot();
	//m_Otherplayer[index]->CreateBoneMatrixSRV(gGameFramework->GetDevice(), cpu, gpu);
	if (index == 0) m_OtherJobNum[0] = 99;
	if (index == 1) m_OtherJobNum[1] = 99;

}
