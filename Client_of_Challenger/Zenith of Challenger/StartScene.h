#pragma once
#include "Scene.h"
#include "GameFramework.h"

class StartScene : public Scene
{
public:
    StartScene();
    ~StartScene() override = default;

    void BuildObjects(const ComPtr<ID3D12Device>& device,
        const ComPtr<ID3D12GraphicsCommandList>& commandList,
        const ComPtr<ID3D12RootSignature>& rootSignature) override;

    void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const override;
    virtual void PreRender(const ComPtr<ID3D12GraphicsCommandList>& commandList);


    virtual void Update(FLOAT timeElapsed);

    virtual void MouseEvent(HWND hWnd, FLOAT timeElapsed);
    virtual void MouseEvent(UINT message, LPARAM lParam);


    virtual void KeyboardEvent(UINT message, WPARAM wParam); 

    virtual void BuildShaders(const ComPtr<ID3D12Device>& device,
        const ComPtr<ID3D12GraphicsCommandList>& commandList,
        const ComPtr<ID3D12RootSignature>& rootSignature);
    virtual void BuildMeshes(const ComPtr<ID3D12Device>& device,
        const ComPtr<ID3D12GraphicsCommandList>& commandList);
    virtual void BuildTextures(const ComPtr<ID3D12Device>& device,
        const ComPtr<ID3D12GraphicsCommandList>& commandList);
    virtual void BuildObjects(const ComPtr<ID3D12Device>& device);

    bool IsStartButtonClicked() const { return m_isStartButtonClicked; }
    void ResetStartButtonClicked() { m_isStartButtonClicked = false; }
    void UpdateLoginObjects();

private:
    //로그인 전용 맴버변수
    string username;
    string password;
    bool isTypingUsername = true;
    bool m_isRoomSelectionActive = false; //방 선택 여부
    bool m_isMouseOnStartBtn = false;
    bool m_isStartButtonClicked = false;

    shared_ptr<GameObject> m_startBtn; // START 버튼 단독 추적용
    vector<bool> m_hasJoinedRoom; // 각 방 참가 여부
};

shared_ptr<Mesh<TextureVertex>> CreateScreenQuad(
    const ComPtr<ID3D12Device>& device,
    const ComPtr<ID3D12GraphicsCommandList>& commandList,
    float width, float height, float z = 0.f);