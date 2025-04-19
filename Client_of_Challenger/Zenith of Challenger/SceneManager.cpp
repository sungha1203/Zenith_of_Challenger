#include "SceneManager.h"
#include "StartScene.h"
#include "GameScene.h"

SceneManager::SceneManager()
    : m_currentScene(nullptr)
{
}

void SceneManager::MouseEvent(HWND hWnd, FLOAT timeElapsed)
{
    m_currentScene->MouseEvent(hWnd, timeElapsed);
}

void SceneManager::KeyboardEvent(FLOAT timeElapsed)
{
    m_currentScene->KeyboardEvent(timeElapsed);
}

void SceneManager::AddScene(const std::string& name, std::shared_ptr<Scene> scene)
{
    m_scenes[name] = scene;
}

void SceneManager::ChangeScene(const std::string& name, const ComPtr<ID3D12Device>& device,
    const ComPtr<ID3D12GraphicsCommandList>& commandList,
    const ComPtr<ID3D12RootSignature>& rootSignature)
{
    if (m_scenes.count(name))
    {
        if (m_currentScene)
        {
            std::cout << "씬 해제 중: " << name << std::endl;

            gGameFramework->WaitForGpuComplete(); // 추가
            
            smotherid[0]= m_currentScene->otherid[0];
            smotherid[1]= m_currentScene->otherid[1];

            smotherpos[0] = m_currentScene->otherpos[0];
            smotherpos[1] = m_currentScene->otherpos[1];
            //모든 리소스 해제
            m_currentScene->ReleaseUploadBuffer();
            m_currentScene->ClearSceneResources(); // 추가적으로 해제

            //GPU 동기화
            device->GetDeviceRemovedReason();
        }

        m_currentScene = m_scenes[name];

        m_currentScene->otherid[0]= smotherid[0];
        m_currentScene->otherid[1]= smotherid[1];

        m_currentScene->otherpos[0]=smotherpos[0];
        m_currentScene->otherpos[1]=smotherpos[1];
        // 새로운 씬에 DirectX 객체 전달
        m_currentScene->SetDevice(device);
        m_currentScene->SetCommandList(commandList);
        m_currentScene->SetRootSignature(rootSignature);

        // 디버깅: 씬 변경 로그 추가
        cout << "씬 변경됨: " << name << endl;

        // BuildObjects가 실행되는지 확인
        if (m_currentScene)
        {
            m_currentScene->BuildObjects(device, commandList, rootSignature);
        }
        else
        {
            cout << "씬이 nullptr입니다!" << endl;
        }
    }
}


void SceneManager::Update(float deltaTime)
{
    if (m_currentScene)
    {
        m_currentScene->Update(deltaTime);
    }
}

void SceneManager::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    if (m_currentScene)
    {
        m_currentScene->PreRender(commandList);
        m_currentScene->Render(commandList);
    }
}

void SceneManager::ReleaseUploadBuffer()
{
    if (m_currentScene)
    {
        m_currentScene->ReleaseUploadBuffer();
    }
}

void SceneManager::Release()
{
    if (m_currentScene)
    {
        m_currentScene.reset();
    }
    m_scenes.clear();
}
