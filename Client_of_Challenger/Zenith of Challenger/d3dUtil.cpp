// d3dUtil.cpp
#include "d3dUtil.h"

#include "d3dUtil.h"

void d3dUtil::CreateDefaultBuffer(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* commandList,
    const void* initData,
    UINT64 byteSize,
    ComPtr<ID3D12Resource>& defaultBuffer,
    ComPtr<ID3D12Resource>& uploadBuffer,
    D3D12_RESOURCE_STATES finalState /* 추가됨 */)
{
    // 1. GPU 용 DefaultHeap 생성
    CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);

    ThrowIfFailed(device->CreateCommittedResource(
        &defaultHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&defaultBuffer)));

    // 2. CPU에서 복사용 UploadHeap 생성
    CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);

    ThrowIfFailed(device->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&uploadBuffer)));

    // 3. 초기 데이터 설정
    D3D12_SUBRESOURCE_DATA subResource = {};
    subResource.pData = initData;
    subResource.RowPitch = byteSize;
    subResource.SlicePitch = subResource.RowPitch;

    // 4. DefaultBuffer를 COPY_DEST로 전이
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        defaultBuffer.Get(),
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_COPY_DEST));

    // 5. 업로드
    UpdateSubresources<1>(
        commandList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResource);

    // 6. 최종 상태로 전이 (매개변수에서 받음)
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        defaultBuffer.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        finalState));
}


