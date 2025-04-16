#pragma once
#include "stdafx.h"

struct VertexBase abstract {};

struct Vertex : public VertexBase
{
	Vertex() = default;
	Vertex(XMFLOAT3 position) : position{ position } {}
	XMFLOAT3 position;
};

struct TextureVertex : public VertexBase
{
	TextureVertex() = default;
	TextureVertex(XMFLOAT3 position, XMFLOAT3 normal, XMFLOAT2 uv) :
		position{ position }, normal{ normal }, uv{ uv } {
	}
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 uv;
};

struct DetailVertex : public VertexBase
{
	DetailVertex() = default;
	DetailVertex(XMFLOAT3 position, XMFLOAT2 uv0, XMFLOAT2 uv1) : position{ position }, uv0{ uv0 }, uv1{ uv1 } {}
	XMFLOAT3 position;
	XMFLOAT2 uv0;
	XMFLOAT2 uv1;
};

struct SkinnedVertex : public VertexBase
{
	SkinnedVertex() = default;
	SkinnedVertex(XMFLOAT3 position, XMFLOAT3 normal, XMFLOAT2 uv, XMUINT4 boneIndices, XMFLOAT4 boneWeights)
		: position{ position }, normal{ normal }, uv{ uv }, boneIndices{ boneIndices }, boneWeights{ boneWeights } {
	}

	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 uv;
	XMUINT4 boneIndices;
	XMFLOAT4 boneWeights;
};

struct DebugVertex : public VertexBase
{
	XMFLOAT3 position;
	DebugVertex() = default;
	DebugVertex(const XMFLOAT3& pos) : position(pos) {}
};