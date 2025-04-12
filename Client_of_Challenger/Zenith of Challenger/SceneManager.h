#pragma once
#include "Scene.h"
#include <unordered_map>

class SceneManager
{
public:
    SceneManager();
    ~SceneManager() = default;

    virtual void MouseEvent(HWND hWnd, FLOAT timeElapsed);
    virtual void KeyboardEvent(FLOAT timeElapsed);

    void AddScene(const string& name, shared_ptr<Scene> scene);
    void ChangeScene(const string& name, const ComPtr<ID3D12Device>& device,
        const ComPtr<ID3D12GraphicsCommandList>& commandList,
        const ComPtr<ID3D12RootSignature>& rootSignature);
    void Update(float deltaTime);
    void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList);
    void ReleaseUploadBuffer();
    void Release();

    shared_ptr<Scene> GetCurrentScene() const { return m_currentScene; }

private:
    unordered_map<string, shared_ptr<Scene>> m_scenes;
    shared_ptr<Scene> m_currentScene;
};