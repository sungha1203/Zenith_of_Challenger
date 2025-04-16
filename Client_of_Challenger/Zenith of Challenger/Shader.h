//-----------------------------------------------------------------------------
// File: Shader.h
//-----------------------------------------------------------------------------
#pragma once
#include "stdafx.h"

class Shader abstract
{
public:
	Shader() = default;
	virtual ~Shader() = default;

	virtual void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList);

protected:
	ComPtr<ID3D12PipelineState> m_pipelineState;
};

class ObjectShader : public Shader
{
public:
	ObjectShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
	~ObjectShader() override = default;
};

class SkyboxShader : public Shader
{
public:
	SkyboxShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
	~SkyboxShader() override = default;
};

class DetailShader : public Shader
{
public:
	DetailShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
	~DetailShader() override = default;
};

// FBX ¸ðµ¨ Àü¿ë ½¦ÀÌ´õ Ãß°¡
class FBXShader : public Shader
{
public:
	FBXShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
	~FBXShader() override = default;
};

class UIScreenShader : public Shader
{
public:
	UIScreenShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
	~UIScreenShader() override = default;
};
class GameSceneUIShader : public Shader
{
public:
	GameSceneUIShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
	~GameSceneUIShader() override = default;
};
class CharacterShader : public Shader
{
public:
	CharacterShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
	~CharacterShader() override = default;
};

class FrightFlyShader : public Shader
{
public:
	FrightFlyShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
	~FrightFlyShader() override = default;
};

class DebugLineShader : public Shader
{
public:
	DebugLineShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
	~DebugLineShader() override = default;
};
