#pragma once

#include "Timer.h"
#include "Player.h"
#include "Scene.h"
#include "SceneManager.h"
#include "GameScene.h"
#include "StartScene.h"
#include "network.h"
#include "ClientState.h"

class ClientNetwork;
class ClientState;

class CGameFramework
{
public:
    CGameFramework(UINT windowWidth, UINT windowHeight);
    ~CGameFramework();

    std::unique_ptr<ClientNetwork>  m_clientNetwork;        // ��Ʈ��ũ ����
    std::unique_ptr<ClientState>    m_clientstate;          // �ΰ��� �� ����
    ClientNetwork* GetClientNetwork() const { return m_clientNetwork.get(); }
    ClientState* GetClientState() const { return m_clientstate.get(); }
    bool IsSuccess = false;                                 // ���ӽ��� ��������?
    XMFLOAT3 g_pos;
    XMFLOAT3 g_pos2;
    XMFLOAT3 g_pos3;
    std::unordered_map<int, XMFLOAT3> monstersCoord;

    void OnCreate(HINSTANCE hInstance, HWND hMainWnd);
    void OnDestroy();

    void FrameAdvance();

    void MouseEvent(HWND hWnd, FLOAT timeElapsed);
    void KeyboardEvent(FLOAT timeElapsed);
    void MouseEvent(UINT message, LPARAM lParam);
    void KeyboardEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    void SetActive(BOOL isActive);

    FLOAT GetAspectRatio();
    UINT GetWindowWidth();
    UINT GetWindowHeight();

    ComPtr<ID3D12Device> GetDevice() { return m_device; }
    ComPtr<ID3D12GraphicsCommandList> GetCommandList() { return m_commandList; }
    ComPtr<ID3D12CommandAllocator> GetCommandAllocator() { return m_commandAllocator; }
    ComPtr<ID3D12CommandQueue> GetCommandQueue() const { return m_commandQueue; }

    // Player ��ü ��������
    shared_ptr<Player> GetPlayer() { return m_player; }

    // Player ��ü ���� (GameScene���� ȣ��)
    void SetPlayer(shared_ptr<Player> player) { m_player = player; }

    // SceneManager�� RootSignature�� ��ȯ�ϴ� �Լ� �߰�
    SceneManager* GetSceneManager() { return m_sceneManager.get(); }
    ComPtr<ID3D12RootSignature> GetRootSignature() { return m_rootSignature; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetCpuSrvHandle() const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuSrvHandle() const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHeapStart() const;
    UINT GetDescriptorSize() const;

    ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() const { return m_cbvSrvUavHeap; }
    ComPtr<ID3D12Fence> GetFence() const { return m_fence; }
    UINT64 GetFenceValue() const { return m_fenceValue; }
    HANDLE GetFenceEvent() const { return m_fenceEvent; }


    std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> AllocateDescriptorHeapSlot();
    UINT GetCurrentSRVOffset() const { return m_srvHeapOffset; }
    HWND GetHWND() const { return m_hWnd; } 

    void WaitForGpuComplete();
private:
    void InitDirect3D();

    void CreateDevice();
    void CreateFence();
    void Check4xMSAAMultiSampleQuality();
    void CreateCommandQueueAndList();
    void CreateSwapChain();
    void CreateRtvDsvDescriptorHeap();
    void CreateRenderTargetView();
    void CreateDepthStencilView();
    void CreateRootSignature();
    void CreateDescriptorHeaps();
    void HandleSceneTransition();

    void BuildObjects();

    void Update();
    void Render();

    void ProcessInput();   // Ű �Է� üũ
private:
    static const UINT m_nSwapChainBuffers = 2;

    BOOL m_activate;

    HINSTANCE m_hInstance;
    HWND m_hWnd;
    UINT m_nWndClientWidth;
    UINT m_nWndClientHeight;
    FLOAT m_aspectRatio;

    D3D12_VIEWPORT m_viewport;
    D3D12_RECT m_scissorRect;
    ComPtr<IDXGIFactory4> m_factory;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12Device> m_device;

    INT m_MSAA4xQualityLevel;
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    ComPtr<ID3D12Resource> m_renderTargets[m_nSwapChainBuffers];
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    UINT m_rtvDescriptorSize;
    ComPtr<ID3D12Resource> m_depthStencil;
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    ComPtr<ID3D12RootSignature> m_rootSignature;

    ComPtr<ID3D12Fence> m_fence;
    UINT m_frameIndex;
    UINT64 m_fenceValue;
    HANDLE m_fenceEvent;

    CGameTimer m_GameTimer;
    POINT m_ptOldCursorPos;

    _TCHAR m_pszBaseTitle[256]; // ���� â ����
    _TCHAR m_pszFrameRate[50]; // FPS ǥ�ÿ�

    unique_ptr<SceneManager> m_sceneManager;
    shared_ptr<Player> m_player;  // �÷��̾� ��ü �߰� GameFramework�� Player�� ����

    bool m_StartButton = false;

    // [SRV �� ���� ��� ���� �߰�]
    ComPtr<ID3D12DescriptorHeap> m_cbvSrvUavHeap;
    D3D12_CPU_DESCRIPTOR_HANDLE m_cbvSrvUavCpuDescriptorStartHandle{};
    D3D12_GPU_DESCRIPTOR_HANDLE m_cbvSrvUavGpuDescriptorStartHandle{};
    UINT m_cbvSrvUavDescriptorSize = 0;

    UINT m_srvHeapOffset = 0;// ������ �� ���� �ε���
    bool m_shouldTransition = false;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //�α��� ����
    std::unordered_map<int, bool> m_keyPressed; // �� Ű �Է� ���� ����


};