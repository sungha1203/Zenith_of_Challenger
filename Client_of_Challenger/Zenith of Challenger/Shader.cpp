//-----------------------------------------------------------------------------
// File: Shader.cpp
//-----------------------------------------------------------------------------

#include "shader.h"

void Shader::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	commandList->SetPipelineState(m_pipelineState.Get());
}

ObjectShader::ObjectShader(const ComPtr<ID3D12Device>& device, 
	const ComPtr<ID3D12RootSignature>& rootSignature)
{
	vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "INSTANCE_ID", 0, DXGI_FORMAT_R32_UINT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }
	};

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ComPtr<ID3DBlob> mvsByteCode, mpsByteCode;
	D3DCompileFromFile(TEXT("Object.hlsl"), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "VERTEX_MAIN", "vs_5_1", compileFlags, 0, &mvsByteCode, nullptr);
	D3DCompileFromFile(TEXT("Object.hlsl"), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "PIXEL_MAIN", "ps_5_1", compileFlags, 0, &mpsByteCode, nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = {
		reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()),
		mvsByteCode->GetBufferSize() };
	psoDesc.PS = {
		reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()),
		mpsByteCode->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
}

SkyboxShader::SkyboxShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature)
{
	vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } };

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ComPtr<ID3DBlob> mvsByteCode, mpsByteCode;
	D3DCompileFromFile(TEXT("SkyBox.hlsl"), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "SKYBOX_VERTEX", "vs_5_1", compileFlags, 0, &mvsByteCode, nullptr);
	D3DCompileFromFile(TEXT("SkyBox.hlsl"), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "SKYBOX_PIXEL", "ps_5_1", compileFlags, 0, &mpsByteCode, nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = {
		reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()),
		mvsByteCode->GetBufferSize() };
	psoDesc.PS = {
		reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()),
		mpsByteCode->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
}

DetailShader::DetailShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature)
{
	vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } };

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ComPtr<ID3DBlob> mvsByteCode, mpsByteCode;
	D3DCompileFromFile(TEXT("Terrain.hlsl"), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "DETAIL_VERTEX", "vs_5_1", compileFlags, 0, &mvsByteCode, nullptr);
	D3DCompileFromFile(TEXT("Terrain.hlsl"), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "DETAIL_PIXEL", "ps_5_1", compileFlags, 0, &mpsByteCode, nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = {
		reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()),
		mvsByteCode->GetBufferSize() };
	psoDesc.PS = {
		reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()),
		mpsByteCode->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
}

FBXShader::FBXShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature)
{
	vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	UINT compileFlags = 0;
#if defined(_DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3DBlob> mvsByteCode, mpsByteCode;
	ComPtr<ID3DBlob> errorMsgs;

	HRESULT hr1 = D3DCompileFromFile(TEXT("FBXShader.hlsl"), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_1", compileFlags, 0, &mvsByteCode, &errorMsgs);

	if (FAILED(hr1))
	{
		if (errorMsgs)
		{
			std::cerr << "FBXShader Vertex Shader 컴파일 오류: " << (char*)errorMsgs->GetBufferPointer() << std::endl;
		}
		else
		{
			std::cerr << "FBXShader Vertex Shader 파일을 찾을 수 없습니다!" << std::endl;
		}
		return;
	}

	HRESULT hr2 = D3DCompileFromFile(TEXT("FBXShader.hlsl"), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_1", compileFlags, 0, &mpsByteCode, &errorMsgs);

	if (FAILED(hr2))
	{
		if (errorMsgs)
		{
			std::cerr << "FBXShader Pixel Shader 컴파일 오류: " << (char*)errorMsgs->GetBufferPointer() << std::endl;
		}
		else
		{
			std::cerr << "FBXShader Pixel Shader 파일을 찾을 수 없습니다!" << std::endl;
		}
		return;
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = { reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()), mvsByteCode->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()), mpsByteCode->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; // 컬링 비활성화
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hr3 = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
	if (FAILED(hr3))
	{
		std::cerr << "FBXShader 파이프라인 생성 실패!" << std::endl;
	}

	if (!m_pipelineState)
		std::cout << "맵 PSO 생성 실패" << std::endl;
	else
		cout << "맵 PSO 생성 성공!!" << endl;

}

UIScreenShader::UIScreenShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature)
{
	vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
	   { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	   { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	   { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ComPtr<ID3DBlob> vsByteCode, psByteCode;
	
		D3DCompileFromFile(TEXT("UIScreenShader.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"VSMain", "vs_5_1", compileFlags, 0, &vsByteCode, nullptr);
		D3DCompileFromFile(TEXT("UIScreenShader.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"PSMain", "ps_5_1", compileFlags, 0, &psByteCode, nullptr);
	

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = { reinterpret_cast<BYTE*>(vsByteCode->GetBufferPointer()), vsByteCode->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<BYTE*>(psByteCode->GetBufferPointer()), psByteCode->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE; // UI에선 깊이 꺼도 됨
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;

	HRESULT hr=device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));

}





CharacterShader::CharacterShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature)
{
	vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	UINT compileFlags = 0;
#if defined(_DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3DBlob> vs, ps, errorMsgs;

	HRESULT hr1 = D3DCompileFromFile(TEXT("Character.hlsl"), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_1", compileFlags, 0, &vs, &errorMsgs);
	if (FAILED(hr1) && errorMsgs)
		std::cerr << "[CharacterShader] VS Error: " << (char*)errorMsgs->GetBufferPointer() << std::endl;

	HRESULT hr2 = D3DCompileFromFile(TEXT("Character.hlsl"), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_1", compileFlags, 0, &ps, &errorMsgs);
	if (FAILED(hr2) && errorMsgs)
		std::cerr << "[CharacterShader] PS Error: " << (char*)errorMsgs->GetBufferPointer() << std::endl;
	
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = { reinterpret_cast<BYTE*>(vs->GetBufferPointer()), vs->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<BYTE*>(ps->GetBufferPointer()), ps->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;

	device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
	if (!m_pipelineState)
		std::cout << "캐릭터 PSO 생성 실패" << std::endl;
	else
		cout << "캐릭터 PSO 생성 성공!!" << endl;
}


FrightFlyShader::FrightFlyShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature)
{
	vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	UINT compileFlags = 0;
#if defined(_DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3DBlob> vs, ps, errorMsgs;

	HRESULT hr1 = D3DCompileFromFile(TEXT("FrightFly.hlsl"), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_1", compileFlags, 0, &vs, &errorMsgs);
	if (FAILED(hr1) && errorMsgs)
		std::cerr << "[FrightFlyShader] VS Error: " << (char*)errorMsgs->GetBufferPointer() << std::endl;

	HRESULT hr2 = D3DCompileFromFile(TEXT("FrightFly.hlsl"), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_1", compileFlags, 0, &ps, &errorMsgs);
	if (FAILED(hr2) && errorMsgs)
		std::cerr << "[FrightFlyShader] PS Error: " << (char*)errorMsgs->GetBufferPointer() << std::endl;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = { reinterpret_cast<BYTE*>(vs->GetBufferPointer()), vs->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<BYTE*>(ps->GetBufferPointer()), ps->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;

	device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
	if (!m_pipelineState)
		std::cout << "FrightFly PSO 생성 실패" << std::endl;
	else
		cout << "FrightFly PSO 생성 성공!!" << endl;
}

DebugLineShader::DebugLineShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature)
{
	vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
	   { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
		 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ComPtr<ID3DBlob> vsByteCode, psByteCode, errors;

	HRESULT hr1 = D3DCompileFromFile(TEXT("LineShader.hlsl"), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_1", compileFlags, 0, &vsByteCode, &errors);
	if (FAILED(hr1) && errors)
		OutputDebugStringA((char*)errors->GetBufferPointer());

	HRESULT hr2 = D3DCompileFromFile(TEXT("LineShader.hlsl"), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_1", compileFlags, 0, &psByteCode, &errors);
	if (FAILED(hr2) && errors)
		OutputDebugStringA((char*)errors->GetBufferPointer());

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };
	psoDesc.pRootSignature = rootSignature.Get();
	// 셰이더 바이트코드
	psoDesc.VS = { reinterpret_cast<BYTE*>(vsByteCode->GetBufferPointer()), vsByteCode->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<BYTE*>(psByteCode->GetBufferPointer()), psByteCode->GetBufferSize() };
	// Rasterizer: CULL 없음, DepthClip 해제
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	// DepthStencil: Depth 무시하고 그리도록 설정
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = TRUE;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	// Blend: 불투명 기본
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	// 선 그리기용
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// 렌더 타겟 포맷
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	// 샘플링
	psoDesc.SampleDesc.Count = 1;

	device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
}

GameSceneUIShader::GameSceneUIShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature)
{
	vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
	   { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	   { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	   { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ComPtr<ID3DBlob> vsByteCode, psByteCode;

	D3DCompileFromFile(TEXT("GameSceneUI.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"VSMain", "vs_5_1", compileFlags, 0, &vsByteCode, nullptr);
	D3DCompileFromFile(TEXT("GameSceneUI.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"PSMain", "ps_5_1", compileFlags, 0, &psByteCode, nullptr);


	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = { reinterpret_cast<BYTE*>(vsByteCode->GetBufferPointer()), vsByteCode->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<BYTE*>(psByteCode->GetBufferPointer()), psByteCode->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE; // UI에선 깊이 꺼도 됨
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;

	HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
}

ShadowMapShader::ShadowMapShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature)
{
	vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ComPtr<ID3DBlob> vsByteCode, psByteCode, errors;

	// [1] VS 컴파일
	HRESULT hr = D3DCompileFromFile(TEXT("ShadowMap.hlsl"), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_1", compileFlags, 0, &vsByteCode, &errors);
	if (FAILED(hr) && errors)
		OutputDebugStringA((char*)errors->GetBufferPointer());

	// [2] PS 컴파일
	hr = D3DCompileFromFile(TEXT("ShadowMap.hlsl"), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_1", compileFlags, 0, &psByteCode, &errors);
	if (FAILED(hr) && errors)
		OutputDebugStringA((char*)errors->GetBufferPointer());

	// [3] PSO 구성
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = { vsByteCode->GetBufferPointer(), vsByteCode->GetBufferSize() };
	psoDesc.PS = { psByteCode->GetBufferPointer(), psByteCode->GetBufferSize() }; // Pixel Shader 연결됨!
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1; // RTV 1개 (linear depth를 출력하기 위해)
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R32_FLOAT; // 깊이를 float로 출력
	psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.RasterizerState.DepthBias = 10000.f;
	psoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
	psoDesc.RasterizerState.DepthBiasClamp = 0.f;

	ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}

DebugShadowShader::DebugShadowShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSig)
{
#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ComPtr<ID3DBlob> vs, ps, err;
	D3DCompileFromFile(L"FullScreenQuadShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"VSMain", "vs_5_1", compileFlags, 0, &vs, &err);
	D3DCompileFromFile(L"FullScreenQuadShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"PSMain", "ps_5_1", compileFlags, 0, &ps, &err);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
	desc.InputLayout = { nullptr, 0 };
	desc.pRootSignature = rootSig.Get();
	desc.VS = { vs->GetBufferPointer(), vs->GetBufferSize() };
	desc.PS = { ps->GetBufferPointer(), ps->GetBufferSize() };
	desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	desc.DepthStencilState.DepthEnable = FALSE;
	desc.DepthStencilState.StencilEnable = FALSE;
	desc.SampleMask = UINT_MAX;
	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	desc.NumRenderTargets = 1;
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;

	ThrowIfFailed(device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&m_pipelineState)));
}

void DebugShadowShader::Render(const ComPtr<ID3D12GraphicsCommandList>& cmdList, D3D12_GPU_DESCRIPTOR_HANDLE shadowSrv)
{
	cmdList->SetPipelineState(m_pipelineState.Get());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->SetGraphicsRootDescriptorTable(RootParameter::ShadowMap, shadowSrv);
	cmdList->DrawInstanced(3, 1, 0, 0); // Fullscreen triangle

}

HealthBarShader::HealthBarShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature)
{
	vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
	   { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
		 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	   { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
		 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	   { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
		 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ComPtr<ID3DBlob> vsByteCode, psByteCode, errors;

	HRESULT hr1 = D3DCompileFromFile(TEXT("HealthBarShader.hlsl"), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_1", compileFlags, 0, &vsByteCode, &errors);
	if (FAILED(hr1) && errors)
		OutputDebugStringA((char*)errors->GetBufferPointer());

	HRESULT hr2 = D3DCompileFromFile(TEXT("HealthBarShader.hlsl"), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_1", compileFlags, 0, &psByteCode, &errors);
	if (FAILED(hr2) && errors)
		OutputDebugStringA((char*)errors->GetBufferPointer());

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = { reinterpret_cast<BYTE*>(vsByteCode->GetBufferPointer()), vsByteCode->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<BYTE*>(psByteCode->GetBufferPointer()), psByteCode->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;

	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;

	device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
}

OutlineShader::OutlineShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature)
{
	// [1] 스키닝용 정점 입력 레이아웃
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		{ "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },  // float3 position
		{ "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },  // float3 normal
		{ "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },  // float2 uv
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT,  0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },  // uint4 boneIndices
		{ "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },  // float4 boneWeights
	};

	// [2] 셰이더 컴파일
#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ComPtr<ID3DBlob> vs, ps, errors;
	HRESULT hr = D3DCompileFromFile(L"OutlineShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"VSMain", "vs_5_1", compileFlags, 0, &vs, &errors);
	if (FAILED(hr) && errors) {
		OutputDebugStringA((char*)errors->GetBufferPointer());
	}

	hr = D3DCompileFromFile(L"OutlineShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"PSMain", "ps_5_1", compileFlags, 0, &ps, &errors);
	if (FAILED(hr) && errors) {
		OutputDebugStringA((char*)errors->GetBufferPointer());
	}

	// [3] PSO 설정
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
	desc.InputLayout = { inputLayout.data(), static_cast<UINT>(inputLayout.size()) };
	desc.pRootSignature = rootSignature.Get();
	desc.VS = { vs->GetBufferPointer(), vs->GetBufferSize() };
	desc.PS = { ps->GetBufferPointer(), ps->GetBufferSize() };

	// 외곽선은 뒷면만 렌더링
	desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	desc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;

	// 깊이 테스트는 하되 깊이값은 쓰지 않음
	desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	desc.DepthStencilState.DepthEnable = TRUE;
	desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	desc.SampleMask = UINT_MAX;
	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	desc.NumRenderTargets = 1;
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	desc.SampleDesc.Count = 1;

	// 파이프라인 생성
	device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&m_pipelineState));
}

ShadowSkinnedShader::ShadowSkinnedShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature)
{
	vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ComPtr<ID3DBlob> vsByteCode, psByteCode, errors;

	// [1] 셰이더 컴파일
	HRESULT hr = D3DCompileFromFile(TEXT("ShadowMap_Skinned.hlsl"), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_1", compileFlags, 0, &vsByteCode, &errors);
	if (FAILED(hr) && errors) OutputDebugStringA((char*)errors->GetBufferPointer());

	hr = D3DCompileFromFile(TEXT("ShadowMap_Skinned.hlsl"), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_1", compileFlags, 0, &psByteCode, &errors);
	if (FAILED(hr) && errors) OutputDebugStringA((char*)errors->GetBufferPointer());

	// [2] PSO 생성
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = { vsByteCode->GetBufferPointer(), vsByteCode->GetBufferSize() };
	psoDesc.PS = { psByteCode->GetBufferPointer(), psByteCode->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R32_FLOAT; // 깊이값을 출력
	psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	psoDesc.SampleDesc.Count = 1;

	// 그림자 아티팩트 방지를 위한 바이어스 설정 (필요시 조정)
	psoDesc.RasterizerState.DepthBias = 10000.f;
	psoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
	psoDesc.RasterizerState.DepthBiasClamp = 0.f;

	ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}

ShadowCharSkinnedShader::ShadowCharSkinnedShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature)
{
	vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ComPtr<ID3DBlob> vsByteCode, psByteCode, errors;

	// [1] 셰이더 컴파일
	HRESULT hr = D3DCompileFromFile(TEXT("ShadowMap_Char_Skinned.hlsl"), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_1", compileFlags, 0, &vsByteCode, &errors);
	if (FAILED(hr) && errors) OutputDebugStringA((char*)errors->GetBufferPointer());

	hr = D3DCompileFromFile(TEXT("ShadowMap_Char_Skinned.hlsl"), nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_1", compileFlags, 0, &psByteCode, &errors);
	if (FAILED(hr) && errors) OutputDebugStringA((char*)errors->GetBufferPointer());

	// [2] PSO 생성
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = { vsByteCode->GetBufferPointer(), vsByteCode->GetBufferSize() };
	psoDesc.PS = { psByteCode->GetBufferPointer(), psByteCode->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R32_FLOAT; // 깊이값을 출력
	psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	psoDesc.SampleDesc.Count = 1;

	// 그림자 아티팩트 방지를 위한 바이어스 설정 (필요시 조정)
	psoDesc.RasterizerState.DepthBias = 10000.f;
	psoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
	psoDesc.RasterizerState.DepthBiasClamp = 0.f;

	ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}
