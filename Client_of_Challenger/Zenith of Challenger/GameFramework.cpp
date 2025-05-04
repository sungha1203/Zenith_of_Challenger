//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "GameFramework.h"
#include <pix.h>
CGameFramework::CGameFramework(UINT windowWidth, UINT windowHeight) :
	m_nWndClientWidth{ windowWidth }, m_nWndClientHeight{ windowHeight },
	m_aspectRatio{ static_cast<FLOAT>(windowWidth) / static_cast<FLOAT>(windowHeight) },
	m_viewport{ 0.f, 0.f, static_cast<FLOAT>(windowWidth), static_cast<FLOAT>(windowHeight), 0.f, 1.f },
	m_scissorRect{ 0, 0, static_cast<LONG>(windowWidth), static_cast<LONG>(windowHeight) },
	m_frameIndex{ 0 }
{
	m_GameTimer.Reset();
}

CGameFramework::~CGameFramework()
{
	OnDestroy();
}

void CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	// 기본 창 제목 저장
	GetWindowText(m_hWnd, m_pszBaseTitle, sizeof(m_pszBaseTitle) / sizeof(TCHAR));

	//-----------[서버]-----------
	m_clientNetwork = std::make_unique<ClientNetwork>();
	m_clientstate = std::make_unique<ClientState>();
	m_clientNetwork->Connect();		// 서버 연결
	//-----------[서버]-----------
	//PIXSetMarker(0, "PIX 캡처 진입");
	//std::this_thread::sleep_for(std::chrono::seconds(1)); // 1초 대기
	InitDirect3D();
	BuildObjects();
}

void CGameFramework::OnDestroy()
{
	WaitForGpuComplete();
	if (m_sceneManager) {
		m_sceneManager->Release();  // 또는 내부 씬 release 처리
	}
}

void CGameFramework::FrameAdvance()
{
	WaitForGpuComplete();

	m_GameTimer.Tick(0); // FPS 측정

	m_srvHeapOffset = 0; // 매 프레임 디스크립터 오프셋 초기화

	FLOAT deltaTime = m_GameTimer.GetElapsedTime();
	deltaTime = max(min(deltaTime, 1.0f / 30.0f), 1.0f / 60.0f);

	// FPS 및 플레이어 위치를 윈도우 타이틀바에 표시
	std::wstringstream titleStream;
	titleStream << L"Zenith of Challenger - FPS: " << m_GameTimer.GetFPS();

	if (m_player)
	{
		XMFLOAT3 playerPos = m_player->GetPosition();
		titleStream << L" | Pos: ("
			<< fixed << setprecision(2)
			<< playerPos.x << ", " << playerPos.y << ", " << playerPos.z << ")";
	}

	POINT mousePoint;
	GetCursorPos(&mousePoint);
	ScreenToClient(m_hWnd, &mousePoint);
	titleStream << L" | Mouse: (" << mousePoint.x << ", " << mousePoint.y << ")";

	SetWindowText(m_hWnd, titleStream.str().c_str());

	Update();

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));


	RenderShadowMap(); // GameScene에서만, 전환 중에는 스킵
	

	Render();

	HandleSceneTransition();  // 안전한 시점에 씬 전환

	if (m_shouldTransition)
	{
		WaitForGpuComplete();
		m_srvHeapOffset = 0;

		ThrowIfFailed(m_commandAllocator->Reset());
		ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

		std::cout << "[GameFramework] GameScene으로 전환 실행\n";

		CreateShadowMapResources();
		m_sceneManager->ChangeScene("GameScene", m_device, m_commandList, m_rootSignature);

		ThrowIfFailed(m_commandList->Close()); // 이게 꼭 필요해
		ID3D12CommandList* lists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(1, lists);
		WaitForGpuComplete();
		{
			CS_Packet_GameReady pkt;
			pkt.type = CS_PACKET_INGAMEREADY;
			pkt.ReadySuccess = true;
			pkt.size = sizeof(pkt);
			gGameFramework->GetClientNetwork()->SendPacket(reinterpret_cast<const char*>(&pkt), pkt.size);
		}

		m_shouldTransition = false;
	}	

}


void CGameFramework::MouseEvent(HWND hWnd, FLOAT timeElapsed)
{
	m_sceneManager->MouseEvent(hWnd, timeElapsed);
}

void CGameFramework::KeyboardEvent(FLOAT timeElapsed)
{
	m_sceneManager->KeyboardEvent(timeElapsed);
}

void CGameFramework::MouseEvent(UINT message, LPARAM lParam)
{
	if (m_sceneManager && m_sceneManager->GetCurrentScene())
	{
		m_sceneManager->GetCurrentScene()->MouseEvent(message, lParam);
		
	}
}

void CGameFramework::KeyboardEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

}

void CGameFramework::SetActive(BOOL isActive)
{
	m_activate = isActive;
}

FLOAT CGameFramework::GetAspectRatio()
{
	return m_aspectRatio;
}

UINT CGameFramework::GetWindowWidth()
{
	return m_nWndClientWidth;
}

UINT CGameFramework::GetWindowHeight()
{
	return m_nWndClientHeight;
}

D3D12_CPU_DESCRIPTOR_HANDLE CGameFramework::GetCpuSrvHandle() const
{
	return m_cbvSrvUavCpuDescriptorStartHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE CGameFramework::GetGpuSrvHandle() const
{
	return m_cbvSrvUavGpuDescriptorStartHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE CGameFramework::GetGPUHeapStart() const
{
	return m_cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart();
}

UINT CGameFramework::GetDescriptorSize() const
{
	return m_cbvSrvUavDescriptorSize;
}

std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> CGameFramework::AllocateDescriptorHeapSlot()
{
	if (m_srvHeapOffset >= 1024) {
		OutputDebugStringA("디스크립터 힙 오버플로우 발생!\n");
		assert(false); // 디버깅 중이면 강제 중단
	}

	UINT offset = m_srvHeapOffset++;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_cbvSrvUavCpuDescriptorStartHandle;
	cpuHandle.ptr += offset * m_cbvSrvUavDescriptorSize;

	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_cbvSrvUavGpuDescriptorStartHandle;
	gpuHandle.ptr += offset * m_cbvSrvUavDescriptorSize;

	return { cpuHandle, gpuHandle };
}

void CGameFramework::RenderShadowMap()
{
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	ID3D12DescriptorHeap* heaps[] = { m_cbvSrvUavHeap.Get() };
	m_commandList->SetDescriptorHeaps(1, heaps);

	m_commandList->RSSetViewports(1, &m_shadowViewport);
	m_commandList->RSSetScissorRects(1, &m_shadowScissorRect);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = { m_shadowRtv };
	m_commandList->OMSetRenderTargets(1, rtvs, FALSE, nullptr);

	float clearColor[] = { 1.0f, 0.0f, 0.0f, 0.0f };
	m_commandList->ClearRenderTargetView(m_shadowRtv, clearColor, 0, nullptr);

	if (auto* scene = dynamic_cast<GameScene*>(m_sceneManager->GetCurrentScene().get()))
	{
		scene->RenderShadowPass(m_commandList);
	}
}


void CGameFramework::InitDirect3D()
{
	CreateDevice();
	CreateFence();
	Check4xMSAAMultiSampleQuality();
	CreateCommandQueueAndList();
	CreateSwapChain();
	CreateRtvDsvDescriptorHeap();
	CreateRenderTargetView();
	CreateDepthStencilView();
	// [추가] SRV 힙 생성
	CreateDescriptorHeaps();
	CreateShadowMapResources();
	CreateRootSignature();
}

void CGameFramework::CreateDevice()
{
	UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG)
	ComPtr<ID3D12Debug> DebugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController)))) {
		DebugController->EnableDebugLayer();
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif
	CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory));

	ComPtr<IDXGIAdapter1> adapter;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_factory->EnumAdapters1(i, &adapter); ++i) {
		DXGI_ADAPTER_DESC1 adapterDesc;
		adapter->GetDesc1(&adapterDesc);
		if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device)))) break;
	}

	if (!m_device) {
		m_factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
		D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device));
	}
}

void CGameFramework::CreateFence()
{
	m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
	m_fenceEvent = CreateEvent(nullptr, false, false, nullptr);
	m_fenceValue = 1;
}

void CGameFramework::Check4xMSAAMultiSampleQuality()
{
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;

	m_device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msQualityLevels, sizeof(msQualityLevels));
	m_MSAA4xQualityLevel = msQualityLevels.NumQualityLevels;

	assert(m_MSAA4xQualityLevel > 0 && "Unexpected MSAA Quality Level");
}

void CGameFramework::CreateCommandQueueAndList()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc{};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
	m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
	m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList));
	// Reset을 호출하기 때문에 Close 상태로 시작
	m_commandList->Close();
}

void CGameFramework::CreateSwapChain()
{
	DXGI_SWAP_CHAIN_DESC sd{};
	sd.BufferDesc.Width = m_nWndClientWidth;
	sd.BufferDesc.Height = m_nWndClientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.SampleDesc.Count = m_MSAA4xQualityLevel > 1 ? 4 : 1;
	sd.SampleDesc.Quality = m_MSAA4xQualityLevel > 1 ? m_MSAA4xQualityLevel - 1 : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = m_nSwapChainBuffers;
	sd.OutputWindow = m_hWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ComPtr<IDXGISwapChain> swapChain;
	m_factory->CreateSwapChain(m_commandQueue.Get(), &sd, &swapChain);
	swapChain.As(&m_swapChain);
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void CGameFramework::CreateRtvDsvDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.NumDescriptors = m_nSwapChainBuffers;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap));
}

void CGameFramework::CreateRenderTargetView()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle{ m_rtvHeap->GetCPUDescriptorHandleForHeapStart() };
	for (UINT i = 0; i < m_nSwapChainBuffers; ++i) {
		m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));
		m_device->CreateRenderTargetView(m_renderTargets[i].Get(), NULL, rtvHeapHandle);
		rtvHeapHandle.Offset(1, m_rtvDescriptorSize);
	}
}

void CGameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC depthStencilDesc{};
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = m_nWndClientWidth;
	depthStencilDesc.Height = m_nWndClientHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.SampleDesc.Count = m_MSAA4xQualityLevel > 1 ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = m_MSAA4xQualityLevel > 1 ? m_MSAA4xQualityLevel - 1 : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear{};
	optClear.Format = DXGI_FORMAT_D32_FLOAT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&optClear,
		IID_PPV_ARGS(m_depthStencil.GetAddressOf()));

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHeapHandle{ m_dsvHeap->GetCPUDescriptorHandleForHeapStart() };
	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;
	m_device->CreateDepthStencilView(m_depthStencil.Get(), &depthStencilViewDesc, dsvHeapHandle);
}

void CGameFramework::CreateRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE descriptorRange[DescriptorRange::Count];
	descriptorRange[DescriptorRange::TextureCube].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);   // t0, space0
	descriptorRange[DescriptorRange::Texture].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 9, 1, 0);       // t1~t9, space0
	descriptorRange[DescriptorRange::BoneMatrix].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 10, 0);   // t10, space0
	descriptorRange[DescriptorRange::ShadowMap].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 11, 0);    // t11, space0
	descriptorRange[DescriptorRange::MonsterBoneMatrix].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 12, 0); // t12
	CD3DX12_ROOT_PARAMETER rootParameter[RootParameter::Count];
	rootParameter[RootParameter::GameObject].InitAsConstantBufferView(0, 0); // b0, space0
	rootParameter[RootParameter::Camera].InitAsConstantBufferView(1, 0);     // b1, space0
	rootParameter[RootParameter::Material].InitAsConstantBufferView(2, 0);   // b2, space0
	rootParameter[RootParameter::Light].InitAsConstantBufferView(3, 0);      // b3, space0
	rootParameter[RootParameter::ShadowCamera].InitAsConstantBufferView(4, 0); // b4, space0
	rootParameter[RootParameter::Instance].InitAsShaderResourceView(0, 1);   // t0, space1

	rootParameter[RootParameter::TextureCube].InitAsDescriptorTable(
		1, &descriptorRange[DescriptorRange::TextureCube], 
		D3D12_SHADER_VISIBILITY_PIXEL); // t0, space0
	rootParameter[RootParameter::Texture].InitAsDescriptorTable(
		1, &descriptorRange[DescriptorRange::Texture], 
		D3D12_SHADER_VISIBILITY_PIXEL);         // t1~t10, space0
	rootParameter[RootParameter::BoneMatrix].InitAsDescriptorTable(
		1, &descriptorRange[DescriptorRange::BoneMatrix], 
		D3D12_SHADER_VISIBILITY_VERTEX);   
	rootParameter[RootParameter::ShadowMap].InitAsDescriptorTable(
		1, &descriptorRange[DescriptorRange::ShadowMap],
		D3D12_SHADER_VISIBILITY_PIXEL);

	rootParameter[RootParameter::LightingMaterial].InitAsConstantBufferView(0, 1); // b0, space1
	rootParameter[RootParameter::LightingLight].InitAsConstantBufferView(0, 2);    // b0, space2
	rootParameter[RootParameter::MonsterBoneMatrix].InitAsDescriptorTable(1, &descriptorRange[DescriptorRange::MonsterBoneMatrix], D3D12_SHADER_VISIBILITY_VERTEX);

	CD3DX12_STATIC_SAMPLER_DESC samplerDesc[2](0);
	samplerDesc[0].ShaderRegister = 0;
	samplerDesc[1].ShaderRegister = 0;
	samplerDesc[1].RegisterSpace = 1;
	samplerDesc[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDesc[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDesc[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDesc[1].Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	samplerDesc[1].MipLODBias = 0.0f;
	samplerDesc[1].MaxAnisotropy = 8.f;
	samplerDesc[1].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	samplerDesc[1].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	samplerDesc[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(
		_countof(rootParameter),
		rootParameter,
		2,
		samplerDesc,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature, error;
	HRESULT hr = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signature,
		&error
	);

	if (FAILED(hr)) {
		OutputDebugStringA((char*)error->GetBufferPointer());
	}

	hr = m_device->CreateRootSignature(
		0,
		signature->GetBufferPointer(),
		signature->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSignature)
	);
}


void CGameFramework::CreateDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1024; // 여유 있게 잡자
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	ThrowIfFailed(m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_cbvSrvUavHeap)));

	m_cbvSrvUavCpuDescriptorStartHandle = m_cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart();
	m_cbvSrvUavGpuDescriptorStartHandle = m_cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart();
	m_cbvSrvUavDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void CGameFramework::HandleSceneTransition()
{
	auto startScene = dynamic_pointer_cast<StartScene>(m_sceneManager->GetCurrentScene());
	//if (startScene && startScene->IsStartButtonClicked())		// 클라 개발
	if (startScene && IsSuccess == true)						// 서버 개발
	{
		m_shouldTransition = true;
		startScene->ResetStartButtonClicked();
	}
}

void CGameFramework::CreateShadowMapResources()
{
	// 1. ShadowMap 텍스처 생성 (선형 Depth 저장용)
	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	float resolution = 2048 * 4;

	texDesc.Width = resolution;
	texDesc.Height = resolution;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE clearVal = {};
	clearVal.Format = DXGI_FORMAT_R32_FLOAT;
	clearVal.Color[0] = 1.0f;
	clearVal.Color[1] = 0.0f;
	clearVal.Color[2] = 0.0f;
	clearVal.Color[3] = 0.0f;

	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		&clearVal,
		IID_PPV_ARGS(&m_shadowMapTexture)));

	// 2. RTV 힙 생성 및 RTV 만들기
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = 1;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_shadowRtvHeap)));

	m_shadowRtv = m_shadowRtvHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_RENDER_TARGET_VIEW_DESC rtvViewDesc = {};
	rtvViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	rtvViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvViewDesc.Texture2D.MipSlice = 0;

	m_device->CreateRenderTargetView(m_shadowMapTexture.Get(), &rtvViewDesc, m_shadowRtv);

	// 3. SRV 생성 (전역 힙 사용)
	D3D12_SHADER_RESOURCE_VIEW_DESC srvViewDesc = {};
	srvViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvViewDesc.Texture2D.MipLevels = 1;

	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart(), m_srvHeapOffset, m_cbvSrvUavDescriptorSize);
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(m_cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart(), m_srvHeapOffset, m_cbvSrvUavDescriptorSize);

	m_device->CreateShaderResourceView(m_shadowMapTexture.Get(), &srvViewDesc, cpuHandle);
	m_shadowSrv = gpuHandle;
	m_srvHeapOffset++;

	// 4. DSV 생성 (선택적: 필요 시 사용)
	D3D12_DESCRIPTOR_HEAP_DESC dsvDesc = {};
	dsvDesc.NumDescriptors = 1;
	dsvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvDesc, IID_PPV_ARGS(&m_shadowDsvHeap)));

	m_shadowDsv = m_shadowDsvHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvViewDesc = {};
	dsvViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	// m_shadowMapTexture는 DSV 용도로 쓰긴 애매함 → 별도 depth 버퍼 쓰는 게 좋음
	m_device->CreateDepthStencilView(nullptr, &dsvViewDesc, m_shadowDsv); // 안 쓰는 경우 null도 가능

	// 5. Viewport / Scissor 설정
	m_shadowViewport = { 0.0f, 0.0f, resolution, resolution, 0.0f, 1.0f };
	m_shadowScissorRect = { 0, 0, (int)resolution, (int)resolution };
}



void CGameFramework::BuildObjects()
{
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	m_sceneManager = make_unique<SceneManager>();

	// StartScene 추가 (스카이박스만 표시)
	auto startScene = make_shared<StartScene>();
	m_sceneManager->AddScene("StartScene", startScene);

	// GameScene 추가 (오브젝트, 터레인, 스카이박스 등 기존 구현 유지) 
	auto gameScene = make_shared<GameScene>();
	m_sceneManager->AddScene("GameScene", gameScene);

	// 기본 씬으로 StartScene 설정
	m_sceneManager->ChangeScene("StartScene", m_device, m_commandList, m_rootSignature);
	//m_sceneManager->ChangeScene("GameScene", m_device, m_commandList, m_rootSignature); //클라 개발용 바로 게임씬 들어가도록

	m_commandList->Close();
	ID3D12CommandList* ppCommandList[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandList), ppCommandList);

	WaitForGpuComplete();

	m_sceneManager->ReleaseUploadBuffer();
	m_GameTimer.Tick();
}

void CGameFramework::WaitForGpuComplete()
{
	const UINT64 fence = m_fenceValue;
	m_commandQueue->Signal(m_fence.Get(), fence);
	++m_fenceValue;

	if (m_fence->GetCompletedValue() < fence) {
		m_fence->SetEventOnCompletion(fence, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void CGameFramework::Update()
{
	if (m_activate) {
		MouseEvent(m_hWnd, m_GameTimer.GetElapsedTime());
		KeyboardEvent(m_GameTimer.GetElapsedTime());
	}

	ProcessInput();

	if (m_sceneManager)
	{
		m_sceneManager->Update(m_GameTimer.GetElapsedTime());

		if (m_player)
			m_player->Update(m_GameTimer.GetElapsedTime());
		if (GetSceneManager()->GetCurrentScene()->m_Otherplayer[0])
			GetSceneManager()->GetCurrentScene()->m_Otherplayer[0]->Update(m_GameTimer.GetElapsedTime());
		if (GetSceneManager()->GetCurrentScene()->m_Otherplayer[1])
			GetSceneManager()->GetCurrentScene()->m_Otherplayer[1]->Update(m_GameTimer.GetElapsedTime());
	}
}


void CGameFramework::Render()
{
	if (!m_sceneManager) return;

	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	ID3D12DescriptorHeap* heaps[] = {
	gGameFramework->GetDescriptorHeap().Get()
	};
	m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);

	// 상태 전이: Present RenderTarget
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET));

	if (m_sceneManager->GetCurrentScene())
		m_sceneManager->GetCurrentScene()->PreRender(m_commandList);

	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle{
		m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
		static_cast<INT>(m_frameIndex), m_rtvDescriptorSize };
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle{
		m_dsvHeap->GetCPUDescriptorHandleForHeapStart()
	};
	m_commandList->OMSetRenderTargets(1, &rtvHandle, TRUE, &dsvHandle);

	const FLOAT clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->ClearDepthStencilView(dsvHandle,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	m_sceneManager->Render(m_commandList);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* commandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	ThrowIfFailed(m_swapChain->Present(0, 0));

	WaitForGpuComplete();
}


void CGameFramework::ProcessInput()
{
	if (GetForegroundWindow() != m_hWnd) return;

	// 메시지 큐를 이용한 입력 처리 최적화
	MSG msg = {};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	// 현재 활성화된 씬이 존재하는지 확인
	auto currentScene = m_sceneManager->GetCurrentScene();
	if (!currentScene)
	{
		std::cout << "[ERROR] 현재 활성화된 씬이 없습니다! 입력을 받을 수 없습니다.\n";
		return;
	}

	// F11 눌렀을 때 전체화면 토글
	if ((GetAsyncKeyState(VK_F11) & 0x8000) && !m_keyPressed[VK_F11])
	{
		ToggleFullScreen();
		m_keyPressed[VK_F11] = true;
	}
	else if (!(GetAsyncKeyState(VK_F11) & 0x8000) && m_keyPressed[VK_F11])
	{
		m_keyPressed[VK_F11] = false;
	}


	// 키 입력이 제대로 감지되지 않을 경우 대비하여 `GetAsyncKeyState` 사용
	for (int key = 0x08; key <= 0xFE; ++key)
	{
		if (key == VK_F11) continue; // 위에서 처리했으니 건너뜀

		SHORT state = GetAsyncKeyState(key);

		bool isDownNow = (state & 0x8000); // 지금 눌려있음
		bool wasDownBefore = m_keyPressed[key];

		if (isDownNow && !wasDownBefore)
		{
			// 새로 눌림 → 이벤트 발생
			currentScene->KeyboardEvent(WM_KEYDOWN, key);
			m_keyPressed[key] = true;
		}
		else if (!isDownNow && wasDownBefore)
		{
			// 키가 떼짐 → 상태만 업데이트 (이벤트 필요 없다면 생략 가능)
			currentScene->KeyboardEvent(WM_KEYUP, key);
			m_keyPressed[key] = false; 
		}
	}

	if (GetAsyncKeyState('Q') & 0x8000) exit(1);


	m_commandList->Close();
	ID3D12CommandList* ppCommandList[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandList), ppCommandList);

	WaitForGpuComplete();
}

void CGameFramework::ToggleFullScreen()
{
	m_isFullScreen = !m_isFullScreen;

	// 현재 윈도우 스타일 가져오기
	DWORD dwStyle = GetWindowLong(m_hWnd, GWL_STYLE);
	if (m_isFullScreen)
	{
		MONITORINFO mi = { sizeof(mi) };
		if (GetWindowPlacement(m_hWnd, &m_wpPrev) &&
			GetMonitorInfo(MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY), &mi))
		{
			SetWindowLong(m_hWnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(m_hWnd, HWND_TOP,
				mi.rcMonitor.left, mi.rcMonitor.top,
				mi.rcMonitor.right - mi.rcMonitor.left,
				mi.rcMonitor.bottom - mi.rcMonitor.top,
				SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}
	else
	{
		SetWindowLong(m_hWnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(m_hWnd, &m_wpPrev);
		SetWindowPos(m_hWnd, nullptr, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
			SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
}
