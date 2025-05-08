//-----------------------------------------------------------------------------
// File: Mesh.cpp
//-----------------------------------------------------------------------------
#include "Mesh.h"

void MeshBase::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList, size_t count) const
{
    commandList->IASetPrimitiveTopology(m_primitiveTopology);
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    commandList->DrawInstanced(m_vertices, static_cast<UINT>(count), 0, 0);
}

void MeshBase::ReleaseUploadBuffer()
{
	if (m_vertexUploadBuffer) m_vertexUploadBuffer.Reset();
}

TerrainMesh::TerrainMesh(const ComPtr<ID3D12Device>& device,
    const ComPtr<ID3D12GraphicsCommandList>& commandList, const wstring& fileName,
    D3D12_PRIMITIVE_TOPOLOGY primitiveTopology)
{
    m_primitiveTopology = primitiveTopology;
    LoadMesh(device, commandList, fileName);
}

FLOAT TerrainMesh::GetHeight(FLOAT x, FLOAT z) const
{
    const XMFLOAT2 range = XMFLOAT2{
        static_cast<FLOAT>(-m_length / 2),
        static_cast<FLOAT>(+m_length / 2) };
    if (range.x > x || range.y < x || range.x > z || range.y < z) return 0.f;
    const size_t nx = static_cast<size_t>(x + m_length / 2);
    const size_t nz = static_cast<size_t>(z + m_length / 2);

    const XMFLOAT2 percentage = XMFLOAT2{ x - floor(x), z - floor(z) };
    if (percentage.x >= percentage.y) {
        return lerp(lerp(m_height[nz][nx], m_height[nz][nx + 1], percentage.x),
            m_height[nz + 1][nx + 1], percentage.y);
    }
    return lerp(lerp(m_height[nz][nx], m_height[nz + 1][nx], percentage.y),
        m_height[nz + 1][nx + 1], percentage.x);
}

void TerrainMesh::LoadMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, const wstring& fileName)
{
    // Open the .raw file
    ifstream in(fileName, ios::binary);
    if (!in.is_open()) {
        throw runtime_error("Failed to open heightmap file.");
    }

    // Determine the dimensions of the heightmap (2 bytes per sample)
    in.seekg(0, ios::end);
    size_t size = static_cast<size_t>(in.tellg());
    in.seekg(0, ios::beg);

    if (size % 2 != 0) {
        throw runtime_error("Heightmap file size is not aligned to 2 bytes.");
    }

    int length = static_cast<int>(sqrt(size / 2)); // 2 bytes per sample
    if (length * length * 2 != size) {
        throw runtime_error("Heightmap file is not a perfect square (16bit).");
    }

    int middle = length / 2;

    // Resize the height vector
    m_height.resize(length, vector<BYTE>(length));

    // Read the heightmap data (2-byte ushort values)
    for (int z = 0; z < length; ++z) {
        for (int x = 0; x < length; ++x) {
            uint16_t value;
            in.read(reinterpret_cast<char*>(&value), sizeof(uint16_t));
            // 정규화: 16bit → 0~255 BYTE로 줄임 (스케일 조정 가능)
            m_height[z][x] = static_cast<BYTE>(value / 256);
        }
    }

    in.close();

    // Vertex creation
    const float delta = 1.f / static_cast<float>(length);
    vector<DetailVertex> vertices;

    const float heightScale = 15.f;

    for (int z = 0; z < length - 1; ++z) {
        for (int x = 0; x < length - 1; ++x) {
            float nx = static_cast<float>(x - middle);
            float nz = static_cast<float>(z - middle);
            float dx = static_cast<float>(x) * delta;
            float dz = 1.f - static_cast<float>(z) * delta;

            float h00 = static_cast<float>(m_height[z][x] * heightScale);
            float h10 = static_cast<float>(m_height[z + 1][x] * heightScale);
            float h11 = static_cast<float>(m_height[z + 1][x + 1] * heightScale);
            float h01 = static_cast<float>(m_height[z][x + 1] * heightScale);

            vertices.emplace_back(XMFLOAT3{ nx, h00, nz },
                XMFLOAT2{ dx, dz }, XMFLOAT2{ 0.f, 1.f });
            vertices.emplace_back(XMFLOAT3{ nx, h10, nz + 1 },
                XMFLOAT2{ dx, dz - delta }, XMFLOAT2{ 0.f, 0.f });
            vertices.emplace_back(XMFLOAT3{ nx + 1, h11, nz + 1 },
                XMFLOAT2{ dx + delta, dz - delta }, XMFLOAT2{ 1.f, 0.f });

            vertices.emplace_back(XMFLOAT3{ nx, h00, nz },
                XMFLOAT2{ dx, dz }, XMFLOAT2{ 0.f, 1.f });
            vertices.emplace_back(XMFLOAT3{ nx + 1, h11, nz + 1 },
                XMFLOAT2{ dx + delta, dz - delta }, XMFLOAT2{ 1.f, 0.f });
            vertices.emplace_back(XMFLOAT3{ nx + 1, h01, nz },
                XMFLOAT2{ dx + delta, dz }, XMFLOAT2{ 1.f, 1.f });
        }
    }

    // Create the vertex buffer
    CreateVertexBuffer(device, commandList, vertices);
}
