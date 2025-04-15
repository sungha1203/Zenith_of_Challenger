//#pragma once
//#include "Mesh.h"
//#include "vertex.h"
//#include "stdafx.h"
//
//class BoundingBoxHelper
//{
//public:
//    BoundingBoxHelper(const ComPtr<ID3D12Device>& device,
//        const ComPtr<ID3D12GraphicsCommandList>& commandList);
//
//    void UpdateBox(const DirectX::BoundingBox& box);
//    void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;
//
//    // 복사 금지
//    BoundingBoxHelper(const BoundingBoxHelper&) = delete;
//    BoundingBoxHelper& operator=(const BoundingBoxHelper&) = delete;
//
//private:
//    std::unique_ptr<Mesh<DebugVertex>> m_lineMesh;
//    std::vector<DebugVertex> m_lineVertices;
//
//    ComPtr<ID3D12Device> m_device;
//    ComPtr<ID3D12GraphicsCommandList> m_commandList;
//};
