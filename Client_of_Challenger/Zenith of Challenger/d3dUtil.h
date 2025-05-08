// d3dUtil.h
#pragma once
#include "stdafx.h"


namespace d3dUtil
{
    // DefaultBuffer 생성 + UploadBuffer에 초기 데이터 복사
    void CreateDefaultBuffer(
        ID3D12Device* device,
        ID3D12GraphicsCommandList* commandList,
        const void* initData,
        UINT64 byteSize,
        ComPtr<ID3D12Resource>& defaultBuffer,
        ComPtr<ID3D12Resource>& uploadBuffer,
        D3D12_RESOURCE_STATES finalState); // 추가된 부분

    void CreateUploadBuffer(
        ID3D12Device* device,
        UINT64 byteSize,
        Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);
}