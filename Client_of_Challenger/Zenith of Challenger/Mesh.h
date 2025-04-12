#pragma once
#include "stdafx.h"
#include "vertex.h"

class MeshBase abstract
{
public:
    MeshBase() = default;
    virtual ~MeshBase() = default;

    virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList, size_t count = 1) const;
    virtual void ReleaseUploadBuffer();

protected:
    UINT                        m_vertices;
    ComPtr<ID3D12Resource>      m_vertexBuffer;
    ComPtr<ID3D12Resource>      m_vertexUploadBuffer;
    D3D12_VERTEX_BUFFER_VIEW    m_vertexBufferView;

    D3D12_PRIMITIVE_TOPOLOGY    m_primitiveTopology;
};

// 기존 파일 로드 방식 + FBX 로드 방식 추가
template <typename T> requires derived_from<T, VertexBase>
class Mesh : public MeshBase
{
public:
    Mesh() = default;

    // 기존 `.raw` 파일 로드 방식 유지
    Mesh(const ComPtr<ID3D12Device>& device,
        const ComPtr<ID3D12GraphicsCommandList>& commandList, const wstring& fileName,
        D3D12_PRIMITIVE_TOPOLOGY primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // FBX 데이터를 `std::vector`로 직접 로드하는 생성자 추가
    Mesh(const ComPtr<ID3D12Device>& device,
        const ComPtr<ID3D12GraphicsCommandList>& commandList,
        const vector<T>& vertices,
        D3D12_PRIMITIVE_TOPOLOGY primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ~Mesh() override = default;

protected:
    virtual void LoadMesh(const ComPtr<ID3D12Device>& device,
        const ComPtr<ID3D12GraphicsCommandList>& commandList, const wstring& fileName);

    void CreateVertexBuffer(const ComPtr<ID3D12Device>& device,
        const ComPtr<ID3D12GraphicsCommandList>& commandList, const vector<T>& vertices);
};

// FBX 데이터 로드를 위한 새로운 생성자 정의
template<typename T> requires derived_from<T, VertexBase>
inline Mesh<T>::Mesh(const ComPtr<ID3D12Device>& device,
    const ComPtr<ID3D12GraphicsCommandList>& commandList,
    const vector<T>& vertices,
    D3D12_PRIMITIVE_TOPOLOGY primitiveTopology)
{
    m_primitiveTopology = primitiveTopology;
    CreateVertexBuffer(device, commandList, vertices);
}

// 기존 `.raw` 파일을 로드하는 생성자 정의
template<typename T> requires derived_from<T, VertexBase>
inline Mesh<T>::Mesh(const ComPtr<ID3D12Device>& device,
    const ComPtr<ID3D12GraphicsCommandList>& commandList, const wstring& fileName,
    D3D12_PRIMITIVE_TOPOLOGY primitiveTopology)
{
    m_primitiveTopology = primitiveTopology;
    LoadMesh(device, commandList, fileName);
}

// 기존 `.raw` 파일을 읽는 함수
template<typename T> requires derived_from<T, VertexBase>
inline void Mesh<T>::LoadMesh(const ComPtr<ID3D12Device>& device,
    const ComPtr<ID3D12GraphicsCommandList>& commandList, const wstring& fileName)
{
    ifstream in(fileName, ios::binary);

    UINT vertexNum;
    in >> vertexNum;

    vector<T> vertices;
    vertices.resize(vertexNum);
    in.read(reinterpret_cast<char*>(vertices.data()), vertexNum * sizeof(T));

    CreateVertexBuffer(device, commandList, vertices);
}

// FBX 데이터를 위한 버텍스 버퍼 생성
template<typename T> requires derived_from<T, VertexBase>
inline void Mesh<T>::CreateVertexBuffer(const ComPtr<ID3D12Device>& device,
    const ComPtr<ID3D12GraphicsCommandList>& commandList,
    const vector<T>& vertices)
{
    m_vertices = static_cast<UINT>(vertices.size());
    const UINT vertexBufferSize = m_vertices * sizeof(T);

    //cout << "버텍스 버퍼 생성 중... 총 버텍스 개수: " << m_vertices << endl;

    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_vertexBuffer));

    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_vertexUploadBuffer));

    D3D12_SUBRESOURCE_DATA vertexData{};
    vertexData.pData = vertices.data();
    vertexData.RowPitch = vertexBufferSize;
    vertexData.SlicePitch = vertexData.RowPitch;

    UpdateSubresources<1>(commandList.Get(),
        m_vertexBuffer.Get(), m_vertexUploadBuffer.Get(), 0, 0, 1, &vertexData);

    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = vertexBufferSize;
    m_vertexBufferView.StrideInBytes = sizeof(T);
}

// 기존 TerrainMesh 그대로 유지
class TerrainMesh : public Mesh<DetailVertex>
{
public:
    TerrainMesh(const ComPtr<ID3D12Device>& device,
        const ComPtr<ID3D12GraphicsCommandList>& commandList, const wstring& fileName,
        D3D12_PRIMITIVE_TOPOLOGY primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ~TerrainMesh() override = default;

    FLOAT GetHeight(FLOAT x, FLOAT z) const;

private:
    void LoadMesh(const ComPtr<ID3D12Device>& device,
        const ComPtr<ID3D12GraphicsCommandList>& commandList, const wstring& fileName) override;

private:
    vector<vector<BYTE>> m_height;
    INT m_length;
    FLOAT m_grid;
};
