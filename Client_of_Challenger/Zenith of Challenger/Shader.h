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
	virtual bool IsShadowShader() const { return false; }

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
	virtual bool IsShadowShader() const { return false; }
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

class ShadowMapShader : public Shader
{
public:
	ShadowMapShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
	~ShadowMapShader() override = default;
	bool IsShadowShader() const override { return true; }
};

class DebugShadowShader : public Shader
{
public:
	DebugShadowShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSig);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& cmdList, D3D12_GPU_DESCRIPTOR_HANDLE shadowSrv);
};


class HealthBarShader : public Shader
{
public:
	HealthBarShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
	~HealthBarShader() override = default;
};

class OutlineShader : public Shader {
public:
	OutlineShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
	~OutlineShader() override = default;
};

class MagicBallShader : public Shader
{
public:
	MagicBallShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
	~MagicBallShader() override = default;
};

class MagicImpactShader : public Shader
{
public:
	MagicImpactShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
	~MagicImpactShader() override = default;
};

class HealingEffectShader : public Shader
{
public:
	HealingEffectShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
	~HealingEffectShader() override = default;
};


class ShadowSkinnedShader : public Shader {
public:
	ShadowSkinnedShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
	~ShadowSkinnedShader() override = default;
	bool IsShadowShader() const override { return true; }
};

class ShadowCharSkinnedShader : public Shader {
public:
	ShadowCharSkinnedShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
	~ShadowCharSkinnedShader() override = default;
	bool IsShadowShader() const override { return true; }
};

