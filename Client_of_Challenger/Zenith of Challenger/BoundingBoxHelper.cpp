//#include "BoundingBoxHelper.h"
//
//BoundingBoxHelper::BoundingBoxHelper(
//    const ComPtr<ID3D12Device>& device,
//    const ComPtr<ID3D12GraphicsCommandList>& commandList)
//    : m_device(device), m_commandList(commandList)
//{
//    m_lineVertices.reserve(24); // 12 lines * 2 points
//}
//
//void BoundingBoxHelper::UpdateBox(const BoundingBox& box)
//{
//    XMFLOAT3 corners[8];
//    box.GetCorners(corners); // 8 corners
//
//    m_lineVertices.clear();
//
//    auto add = [&](int a, int b) {
//        m_lineVertices.emplace_back(DebugVertex(corners[a]));
//        m_lineVertices.emplace_back(DebugVertex(corners[b]));
//        };
//
//    // 12 lines
//    add(0, 1); add(1, 2); add(2, 3); add(3, 0); // bottom
//    add(4, 5); add(5, 6); add(6, 7); add(7, 4); // top
//    add(0, 4); add(1, 5); add(2, 6); add(3, 7); // sides
//
//    m_lineMesh = std::make_unique<Mesh<DebugVertex>>(
//        m_device, m_commandList, m_lineVertices, D3D_PRIMITIVE_TOPOLOGY_LINELIST);
//}
//
//void BoundingBoxHelper::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
//{
//    if (m_lineMesh)
//        m_lineMesh->Render(commandList);
//}
