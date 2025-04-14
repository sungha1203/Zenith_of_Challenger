#include "StartScene.h"

std::shared_ptr<Mesh<TextureVertex>> CreateScreenQuad(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, float width, float height, float z)
{
    std::vector<TextureVertex> vertices =
    {
        { XMFLOAT3{-width / 2,  height / 2, z}, XMFLOAT3{0, 0, -1}, XMFLOAT2{0, 0} },
        { XMFLOAT3{ width / 2,  height / 2, z}, XMFLOAT3{0, 0, -1}, XMFLOAT2{1, 0} },
        { XMFLOAT3{-width / 2, -height / 2, z}, XMFLOAT3{0, 0, -1}, XMFLOAT2{0, 1} },

        { XMFLOAT3{-width / 2, -height / 2, z}, XMFLOAT3{0, 0, -1}, XMFLOAT2{0, 1} },
        { XMFLOAT3{ width / 2,  height / 2, z}, XMFLOAT3{0, 0, -1}, XMFLOAT2{1, 0} },
        { XMFLOAT3{ width / 2, -height / 2, z}, XMFLOAT3{0, 0, -1}, XMFLOAT2{1, 1} },
    };

    return std::make_shared<Mesh<TextureVertex>>(device, commandList, vertices);
}


const std::string correctUsername = "E";
const std::string correctPassword = "1";

StartScene::StartScene()
    : isTypingUsername(true)
{
    username = "";
    password = "";
}

void StartScene::BuildObjects(const ComPtr<ID3D12Device>& device,
    const ComPtr<ID3D12GraphicsCommandList>& commandList,
    const ComPtr<ID3D12RootSignature>& rootSignature)
{
    std::cout << "==== 로그인 화면 ====" << std::endl;

    m_meshes.clear();
    m_textures.clear();
    m_objects.clear();
    m_StartSceneObjects.clear();

    BuildShaders(device, commandList, rootSignature);
    BuildMeshes(device, commandList);
    BuildTextures(device, commandList);

    BuildObjects(device);
}

void StartScene::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
    m_camera->UpdateShaderVariable(commandList);

    m_shaders.at("SKYBOX")->UpdateShaderVariable(commandList);
    // 스카이박스 렌더링
    if (m_skybox)
    {
        m_skybox->Render(commandList);
    }

    if (!m_isRoomSelectionActive) {
        if (m_shaders.contains("UI"))
        {
            m_shaders.at("UI")->UpdateShaderVariable(commandList);
            for (const auto& obj : m_StartSceneObjects)
            {
                obj->Render(commandList); // 배경 → 타이틀 순서대로 push_back된 상태
            }
            for (const auto& obj : m_StartSceneObjects) obj->Render(commandList);
            for (const auto& obj : m_idObjects) obj->Render(commandList);
            for (const auto& obj : m_pwObjects) obj->Render(commandList);
        }
    }
    else {
        if (m_shaders.contains("UI"))
        {
            m_shaders.at("UI")->UpdateShaderVariable(commandList);
            for (const auto& obj : m_SelectSceneObjects)
            {
                obj->Render(commandList);
            }

            for (const auto& obj : m_joinButtons)
            {
                obj->Render(commandList);
            }

            if (m_startBtn->IsVisible()) {
                m_startBtn->Render(commandList);
            }
        }
    }

    if (m_isMouseOnStartBtn) {
        if (m_shaders.contains("UI"))
        {
            m_shaders.at("UI")->UpdateShaderVariable(commandList);
            for (const auto& obj : m_startBar)
            {
                obj->Render(commandList);
            }
        }
    }
}

void StartScene::PreRender(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
}

void StartScene::Update(FLOAT timeElapsed)
{
    if (!m_isRoomSelectionActive && gGameFramework->GetClientState()->GetIsLogin())
    {
        m_isRoomSelectionActive = true;
    }
    m_skybox->SetPosition(m_camera->GetEye());

}

void StartScene::MouseEvent(HWND hWnd, FLOAT timeElapsed)
{
}

void StartScene::MouseEvent(UINT message, LPARAM lParam)
{
    if (!m_isRoomSelectionActive) return;

    int mouseX = LOWORD(lParam);
    int mouseY = HIWORD(lParam);

    // ------------------------------------------------------------------
    // START 버튼 클릭 감지 (정확한 World 기준)
    if (m_startBtn && m_startBtn->IsVisible())
    {
        if (mouseX >= 1156 && mouseX <= 1385 &&
            mouseY >= 641 && mouseY <= 675)
        {
            m_isMouseOnStartBtn = true;

            if (message == WM_LBUTTONDOWN)
            {
                std::cout << "START 버튼 클릭됨 → GameScene 전환 예정\n";
                m_isStartButtonClicked = true;
                {
                    CS_Packet_GameStart pkt;
                    pkt.type = CS_PACKET_GAMESTART;
                    pkt.size = sizeof(pkt);
                    gGameFramework->GetClientNetwork()->SendPacket(reinterpret_cast<const char*>(&pkt), pkt.size);
                }
            }
        }
        else
        {
            m_isMouseOnStartBtn = false;
        }
    }

    // ------------------------------------------------------------------
    // JOIN 버튼 Hover 초기화
    for (int i = 0; i < 3; ++i)
        m_joinButtons[i]->SetHovered(false);

    // JOIN 버튼 Hover 감지
    if (mouseX >= 570 && mouseX <= 760)
    {
        for (int i = 0; i < 3; ++i)
        {
            int top = 0, bottom = 0;
            if (i == 0) { top = 107; bottom = 167; }
            if (i == 1) { top = 332; bottom = 393; }
            if (i == 2) { top = 555; bottom = 618; }

            if (mouseY >= top && mouseY <= bottom)
            {
                m_joinButtons[i]->SetHovered(true);

                if (message == WM_LBUTTONDOWN)
                {
                    {
                        CS_Packet_Room pkt;
                        pkt.room_id = i;
                        pkt.type = CS_PACKET_ROOM;
                        pkt.size = sizeof(pkt);

                        gGameFramework->GetClientNetwork()->SendPacket(reinterpret_cast<const char*>(&pkt), pkt.size);
                    }
                    for (int j = 0; j < 3; ++j)
                        m_joinButtons[j]->SetVisible(false);

                    m_startBtn->SetVisible(true); // START 버튼 표시
                }

                break;
            }
        }
    }
}


void StartScene::KeyboardEvent(UINT message, WPARAM wParam)
{
    if (message != WM_KEYDOWN) return;

    if (wParam == VK_BACK)
    {
        if (isTypingUsername && !username.empty()) username.pop_back();
        else if (!isTypingUsername && !password.empty()) password.pop_back();
    }
    else if (wParam == VK_TAB)
    {
        isTypingUsername = !isTypingUsername;
    }
    else if (wParam == VK_RETURN)
    //------------------------------------------------------
    //{
    //    if (username == "ADMIN" && password == "PASS"){
    //        m_isRoomSelectionActive = true; // 씬 전환
    //    }
    //    else {
    //        username.clear();
    //        password.clear();
    //        isTypingUsername = true;
    //    }
    //}
    //------------------------------------------------------
    {
        std::string idpw = username + " " + password;
        char sendBuffer[256] = { 0 };
        sendBuffer[0] = CS_PACKET_LOGIN;
        memcpy(sendBuffer + 1, idpw.c_str(), idpw.length());
        if (gGameFramework && gGameFramework->GetClientNetwork()) {
            gGameFramework->GetClientNetwork()->SendPacket(sendBuffer, 1 + static_cast<int>(idpw.length()));
        }
        username.clear();
        password.clear();
        isTypingUsername = true;
    }
    //------------------------------------------------------
    else if ((wParam >= 'a' && wParam <= 'z') || (wParam >= 'A' && wParam <= 'Z') || (wParam >= '0' && wParam <= '9'))
    {
        char ch = static_cast<char>(wParam);
        if (isTypingUsername) username += ch;
        else password += ch;
    }

    UpdateLoginObjects(); // 화면 반영
}



//void StartScene::KeyboardEvent(FLOAT timeElapsed)
//{
//}

void StartScene::BuildShaders(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, const ComPtr<ID3D12RootSignature>& rootSignature)
{
    auto skyboxShader = make_shared<SkyboxShader>(device, rootSignature);
    m_shaders.insert({ "SKYBOX", skyboxShader });

    auto uiShader = make_shared<UIScreenShader>(device, rootSignature);
    m_shaders.insert({ "UI", uiShader });
}

void StartScene::BuildMeshes(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    auto skyboxMesh = make_shared<Mesh<Vertex>>(device, commandList, TEXT("Model/SkyboxMesh.binary"));
    m_meshes.insert({ "SKYBOX", skyboxMesh });
}

void StartScene::BuildTextures(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    auto skyboxTexture = make_shared<Texture>(device, commandList,
        TEXT("Skybox/SkyBox_0.dds"), RootParameter::TextureCube);
    m_textures.insert({ "SKYBOX", skyboxTexture });

    auto startTex = make_shared<Texture>(device);
    startTex->LoadTexture(device, commandList, TEXT("Image/StartScene/StartScreen.dds"), RootParameter::Texture);
    startTex->CreateShaderVariable(device);
    m_textures.insert({ "START", startTex });

    auto titleTex = make_shared<Texture>(device);
    titleTex->LoadTexture(device, commandList, TEXT("Image/StartScene/Title_transparent.dds"), RootParameter::Texture);
    titleTex->CreateShaderVariable(device);
    m_textures.insert({ "TITLE", titleTex });

    // Room 선택 화면용
    auto roomTex = make_shared<Texture>(device);
    roomTex->LoadTexture(device, commandList, TEXT("Image/Select_Server/Select_Room.dds"), RootParameter::Texture);
    roomTex->CreateShaderVariable(device);
    m_textures.insert({ "ROOM", roomTex });

    auto startBtnTex = make_shared<Texture>(device);
    startBtnTex->LoadTexture(device, commandList, TEXT("Image/Select_Server/START_Button.dds"), RootParameter::Texture);
    startBtnTex->CreateShaderVariable(device);
    m_textures.insert({ "STARTBTN", startBtnTex });

    auto blurOverlayTex = make_shared<Texture>(device);
    blurOverlayTex->LoadTexture(device, commandList, TEXT("Image/Select_Server/Blur.dds"), RootParameter::Texture);
    blurOverlayTex->CreateShaderVariable(device);
    m_textures.insert({ "BLUR", blurOverlayTex });

    auto StartBar = make_shared<Texture>(device);
    StartBar->LoadTexture(device, commandList, TEXT("Image/Select_Server/BAR.dds"), RootParameter::Texture);
    StartBar->CreateShaderVariable(device);
    m_textures.insert({ "BAR", StartBar });

    // 폰트 텍스처 추가
    auto fontTexture = make_shared<Texture>(device);
    fontTexture->LoadTexture(device, commandList, TEXT("Image/NanumFontSprite.dds"), RootParameter::Texture);
    fontTexture->CreateShaderVariable(device);
    m_textures.insert({ "FONT", fontTexture });

    auto tex = make_shared<Texture>(device);
    tex->LoadTexture(device, commandList, TEXT("Image/StartScene/ID.dds"), RootParameter::Texture);
    tex->CreateShaderVariable(device);
    m_textures.insert({ "ID_LABEL", tex });

    tex = make_shared<Texture>(device);
    tex->LoadTexture(device, commandList, TEXT("Image/StartScene/PASSWORD.dds"), RootParameter::Texture);
    tex->CreateShaderVariable(device);
    m_textures.insert({ "PW_LABEL", tex });

    tex = make_shared<Texture>(device);
    tex->LoadTexture(device, commandList, TEXT("Image/StartScene/WORD.dds"), RootParameter::Texture);
    tex->CreateShaderVariable(device);
    m_textures.insert({ "WORD", tex });

    tex = make_shared<Texture>(device);
    tex->LoadTexture(device, commandList, TEXT("Image/StartScene/STAR.dds"), RootParameter::Texture);
    tex->CreateShaderVariable(device);
    m_textures.insert({ "STAR", tex });

    auto JoinTexture = make_shared<Texture>(device);
    JoinTexture->LoadTexture(device, commandList, TEXT("Image/Select_Server/Join_converted.dds"), RootParameter::Texture);
    JoinTexture->CreateShaderVariable(device);
    m_textures.insert({ "JOIN", JoinTexture });

    auto FirstRoomTexture = make_shared<Texture>(device);
    FirstRoomTexture->LoadTexture(device, commandList, TEXT("Image/Select_Server/1번방.dds"), RootParameter::Texture);
    FirstRoomTexture->CreateShaderVariable(device);
    m_textures.insert({ "1번방", FirstRoomTexture });

    auto SecondRoomTexture = make_shared<Texture>(device);
    SecondRoomTexture->LoadTexture(device, commandList, TEXT("Image/Select_Server/2번방.dds"), RootParameter::Texture);
    SecondRoomTexture->CreateShaderVariable(device);
    m_textures.insert({ "2번방", SecondRoomTexture });

    auto ThirdRoomTexture = make_shared<Texture>(device);
    ThirdRoomTexture->LoadTexture(device, commandList, TEXT("Image/Select_Server/3번방.dds"), RootParameter::Texture);
    ThirdRoomTexture->CreateShaderVariable(device);
    m_textures.insert({ "3번방", ThirdRoomTexture });

}

void StartScene::BuildObjects(const ComPtr<ID3D12Device>& device)
{
    float screenWidth = 2.0f;
    float screenAspect = static_cast<float>(FRAME_BUFFER_WIDTH) / FRAME_BUFFER_HEIGHT;
    float screenHeight = screenWidth / screenAspect; // 예: 2.0 / 1.8  1.11

    m_camera = make_shared<ThirdPersonCamera>(device);
    m_camera->SetLens(0.25f * XM_PI, screenWidth / screenHeight, 0.1f, 1000.0f);

    m_skybox = make_shared<GameObject>(device);
    m_skybox->SetMesh(m_meshes["SKYBOX"]);
    m_skybox->SetTexture(m_textures["SKYBOX"]);


    // StartScreen: 화면 가득 채우기
    auto screen = make_shared<GameObject>(device);

    screen->SetMesh(CreateScreenQuad(device, gGameFramework->GetCommandList(), 1.1f, 0.9f, 0.99f));
    screen->SetTexture(m_textures["START"]);
    screen->SetUseTexture(true);
    screen->SetBaseColor(XMFLOAT4(1, 1, 1, 1));
    m_StartSceneObjects.push_back(screen);

    // Title (로고 이미지): 앞에 출력, 살짝 위쪽
    auto title = make_shared<GameObject>(device);
    title->SetMesh(CreateScreenQuad(device, gGameFramework->GetCommandList(), 1.8f, 1.6f, 0.98f));
    title->SetTexture(m_textures["TITLE"]);
    title->SetUseTexture(true);
    title->SetBaseColor(XMFLOAT4(1, 1, 1, 1));
    title->SetPosition(XMFLOAT3(0.f, 0.2f, 0.98f));
    m_StartSceneObjects.push_back(title);

    // Room 선택 UI 구성
    // 블러 오버레이
    auto blur = make_shared<GameObject>(device);
    blur->SetMesh(CreateScreenQuad(device, gGameFramework->GetCommandList(), 1.1f, 0.9f, 0.99f));
    blur->SetTexture(m_textures["BLUR"]);
    blur->SetUseTexture(true);
    blur->SetBaseColor(XMFLOAT4(1, 1, 1, 1));
    m_SelectSceneObjects.push_back(blur);

    float roomWidth = 1.0f;
    float roomHeight = 0.2f;

    for (int i = 0; i < 3; ++i)
    {
        auto room = make_shared<GameObject>(device);
        room->SetMesh(CreateScreenQuad(device, gGameFramework->GetCommandList(), 1.0f, 0.5f, 0.98f));
        room->SetTexture(m_textures["ROOM"]);
        room->SetScale(XMFLOAT3(1.5f, 2.0f, 1.2f));
        room->SetPosition(XMFLOAT3(-0.5f, 0.6f - 0.55f * i, 0.98f));
        room->SetUseTexture(true);
        m_SelectSceneObjects.push_back(room);
    }

    m_startBtn = make_shared<GameObject>(device);
    m_startBtn->SetMesh(CreateScreenQuad(device, gGameFramework->GetCommandList(), 0.5f, 0.15f, 0.98f));
    m_startBtn->SetTexture(m_textures["STARTBTN"]);
    m_startBtn->SetScale(XMFLOAT3(0.8f, 1.4f, 1.0f));
    m_startBtn->SetPosition(XMFLOAT3(0.85f, -0.65f, 0.98f));
    m_startBtn->SetUseTexture(true);
    m_startBtn->SetVisible(false); // 처음엔 안 보이게

    auto StartBar = make_shared<GameObject>(device);
    StartBar->SetMesh(CreateScreenQuad(device, gGameFramework->GetCommandList(), 0.5f, 0.15f, 0.98f));
    StartBar->SetTexture(m_textures["BAR"]);
    StartBar->SetScale(XMFLOAT3(0.8f, 1.1f, 1.0f));
    StartBar->SetPosition(XMFLOAT3(0.85f, -0.75f, 0.98f));
    StartBar->SetUseTexture(true);
    m_startBar.push_back(StartBar);

    auto idLabel = make_shared<GameObject>(device);
    idLabel->SetMesh(CreateScreenQuad(device, gGameFramework->GetCommandList(), 0.5f, 0.2f, 0.98f));
    idLabel->SetTexture(m_textures["ID_LABEL"]);
    idLabel->SetUseTexture(true);
    idLabel->SetScale(XMFLOAT3(1.1f, 1.2f, 1.1f));
    idLabel->SetPosition(XMFLOAT3(-0.205f, -0.38f, 0.98f));
    m_StartSceneObjects.push_back(idLabel);

    auto pwLabel = make_shared<GameObject>(device);
    pwLabel->SetMesh(CreateScreenQuad(device, gGameFramework->GetCommandList(), 0.7f, 0.25f, 0.98f));
    pwLabel->SetTexture(m_textures["PW_LABEL"]);
    pwLabel->SetUseTexture(true);
    pwLabel->SetScale(XMFLOAT3(0.7f, 0.8f, 0.7f));
    pwLabel->SetPosition(XMFLOAT3(-0.2f, -0.46f, 0.98f));
    m_StartSceneObjects.push_back(pwLabel);


    for (int i = 0; i < 3; ++i)
    {
        // 참가 버튼
        auto joinBtn = make_shared<GameObject>(device);
        joinBtn->SetMesh(CreateScreenQuad(device, gGameFramework->GetCommandList(), 0.3f, 0.15f, 0.98f));
        joinBtn->SetTexture(m_textures["JOIN"]);
        //joinBtn->SetScale(XMFLOAT3(0.7f, 0.7f, 0.7f));
        joinBtn->SetUseTexture(true);

        // ROOM 우측 위에 위치 (ROOM보다 오른쪽 + 살짝 위)
        joinBtn->SetPosition(XMFLOAT3(-0.07f, 0.5f - 0.5f * i, 0.98f));
        m_joinButtons.push_back(joinBtn);
    }

    auto FirstRoom = make_shared<GameObject>(device);
    FirstRoom->SetMesh(CreateScreenQuad(device, gGameFramework->GetCommandList(), 1.0f, 0.5f, 0.98f));
    FirstRoom->SetTexture(m_textures["1번방"]);
    FirstRoom->SetScale(XMFLOAT3(0.2f, 0.2f, 0.2f));
    FirstRoom->SetPosition(XMFLOAT3(-0.47f, 0.25f, 0.98f));
    FirstRoom->SetUseTexture(true);
    m_SelectSceneObjects.push_back(FirstRoom);

    auto SecondRoom = make_shared<GameObject>(device);
    SecondRoom->SetMesh(CreateScreenQuad(device, gGameFramework->GetCommandList(), 1.0f, 0.5f, 0.98f));
    SecondRoom->SetTexture(m_textures["2번방"]);
    SecondRoom->SetScale(XMFLOAT3(0.2f, 0.2f, 0.2f));
    SecondRoom->SetPosition(XMFLOAT3(-0.47f, -0.05f, 0.98f));
    SecondRoom->SetUseTexture(true);
    m_SelectSceneObjects.push_back(SecondRoom);

    auto ThirdRoom = make_shared<GameObject>(device);
    ThirdRoom->SetMesh(CreateScreenQuad(device, gGameFramework->GetCommandList(), 1.0f, 0.5f, 0.98f));
    ThirdRoom->SetTexture(m_textures["3번방"]);
    ThirdRoom->SetScale(XMFLOAT3(0.2f, 0.2f, 0.2f));
    ThirdRoom->SetPosition(XMFLOAT3(-0.47f, -0.35f, 0.98f));
    ThirdRoom->SetUseTexture(true);
    m_SelectSceneObjects.push_back(ThirdRoom);
}

void StartScene::UpdateLoginObjects()
{
    m_idObjects.clear();
    m_pwObjects.clear();

    float IDStart = -0.03f;
    float PAStart = -0.03f;
    float charWidth = 0.5f;
    float charHeight = 0.25f;

    //// ===== 아이디 처리 =====
    //for (size_t i = 0; i < username.size(); ++i)
    //{
    //    char ch = static_cast<char>(toupper(username[i]));
    //    if (ch < 'A' || ch > 'Z') continue; // 알파벳만 처리

    //    int index = ch - 'A';              // 0~25
    //    float u = index / 26.0f;
    //    float uStep = 1.0f / 26.0f;

    //    XMFLOAT3 normal = { 0.0f, 0.0f, -1.0f };

    //    vector<TextureVertex> vertices = {
    //        { { -0.5f, +0.5f, 0.f }, normal, { u, 0.f } },
    //        { { +0.5f, +0.5f, 0.f }, normal, { u + uStep, 0.f } },
    //        { { -0.5f, -0.5f, 0.f }, normal, { u, 1.f } },
    //        { { +0.5f, -0.5f, 0.f }, normal, { u + uStep, 1.f } },
    //    };

    //    auto mesh = make_shared<Mesh<TextureVertex>>(gGameFramework->GetDevice(), gGameFramework->GetCommandList(), vertices);
    //    auto obj = make_shared<GameObject>(gGameFramework->GetDevice());
    //    obj->SetMesh(mesh);
    //    obj->SetTexture(m_textures["WORD"]);
    //    obj->SetUseTexture(true);
    //    obj->SetScale(XMFLOAT3(0.1f, 0.1f, 0.1f));
    //    obj->SetBaseColor({ 1.f, 1.f, 1.f, 1.f });
    //    obj->SetPosition(XMFLOAT3(IDStart + i * (0.05f), -0.18f, 0.98f));

    //    m_idObjects.push_back(obj);
    //}

    for (size_t i = 0; i < username.size(); ++i)
    {
        auto obj = make_shared<GameObject>(gGameFramework->GetDevice());
        obj->SetMesh(CreateScreenQuad(gGameFramework->GetDevice(), gGameFramework->GetCommandList(), charWidth, charHeight, 0.98f));
        obj->SetTexture(m_textures["STAR"]);
        obj->SetUseTexture(true);
        obj->SetPosition(XMFLOAT3(IDStart + i * (0.05f), -0.37f, 0.98f));
        m_idObjects.push_back(obj);
    }

    for (size_t i = 0; i < password.size(); ++i)
    {
        auto obj = make_shared<GameObject>(gGameFramework->GetDevice());
        obj->SetMesh(CreateScreenQuad(gGameFramework->GetDevice(), gGameFramework->GetCommandList(), charWidth, charHeight, 0.98f));
        obj->SetTexture(m_textures["STAR"]);
        obj->SetUseTexture(true);
        obj->SetPosition(XMFLOAT3(PAStart + i * (0.05f), -0.54f, 0.98f));
        m_pwObjects.push_back(obj);
    }
}
