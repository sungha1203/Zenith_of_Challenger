#pragma once
#include "stdafx.h"
#include "buffer.h"

struct CameraData : public BufferBase
{
	XMFLOAT4X4 viewMatrix;
	XMFLOAT4X4 projectionMatrix;
	XMFLOAT3 eye;
};

class Camera
{
public:
	Camera(const ComPtr<ID3D12Device>& device);
	~Camera() = default;

	virtual void Update(FLOAT timeElapsed) = 0;
	virtual void UpdateEye(XMFLOAT3 position) = 0;
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList);

	virtual void RotatePitch(FLOAT radian) = 0;
	virtual void RotateYaw(FLOAT radian) = 0;

	void SetLens(FLOAT fovy, FLOAT aspect, FLOAT minZ, FLOAT maxZ);
	// Camera.h (protected or public에 추가)
	void SetPosition(const XMFLOAT3& position);
	void SetLookAt(const XMFLOAT3& lookAt);

	XMFLOAT3 GetEye() const;
	XMFLOAT3 GetU() const;
	XMFLOAT3 GetV() const;
	XMFLOAT3 GetN() const;

	virtual void ZoomIn() = 0;
	virtual void ZoomOut() = 0;

protected:
	void UpdateBasis();

protected:
	XMFLOAT4X4 m_viewMatrix;
	XMFLOAT4X4 m_projectionMatrix;

	XMFLOAT3 m_eye;
	XMFLOAT3 m_at;
	XMFLOAT3 m_up;

	XMFLOAT3 m_u;
	XMFLOAT3 m_v;
	XMFLOAT3 m_n;

	unique_ptr<UploadBuffer<CameraData>> m_constantBuffer;
};

class ThirdPersonCamera : public Camera
{
public:
	ThirdPersonCamera(const ComPtr<ID3D12Device>& device);
	~ThirdPersonCamera() = default;

	void Update(FLOAT timeElapsed) override;
	void UpdateEye(XMFLOAT3 position) override;

	void RotatePitch(FLOAT radian) override;
	void RotateYaw(FLOAT radian) override;

	void ZoomIn() override;
	void ZoomOut() override;
private:
	FLOAT m_radius;
	FLOAT m_phi;
	FLOAT m_theta;
};


class QuarterViewCamera : public Camera
{
public:
	QuarterViewCamera(const ComPtr<ID3D12Device>& device);
	~QuarterViewCamera() = default;

	void Update(FLOAT timeElapsed) override;
	void UpdateEye(XMFLOAT3 position) override;

	void RotatePitch(FLOAT radian) override;
	void RotateYaw(FLOAT radian) override;

	void ZoomIn() override;
	void ZoomOut() override;

private:
	FLOAT m_radius;  // 카메라 거리 (줌 기능)
	FLOAT m_phi;     // 카메라의 위/아래 각도 (쿼터뷰 고정)
	FLOAT m_theta;   // 카메라의 좌/우 회전 (플레이어 따라감)

	XMFLOAT3 m_offset; // 플레이어와의 상대적 위치
};

class TopViewCamera : public Camera
{
public:
	TopViewCamera(const ComPtr<ID3D12Device>& device)
		: Camera(device), m_height(50.0f), m_distance(1.0f) {
	}
	~TopViewCamera() = default;

	void Update(FLOAT timeElapsed) override {}
	void UpdateEye(XMFLOAT3 position) override
	{
		SetPosition(XMFLOAT3(position.x, position.y + m_height, position.z));
		SetLookAt(position);
	}

	void RotatePitch(FLOAT radian) override {}
	void RotateYaw(FLOAT radian) override {}

	void ZoomIn() override { m_height = max(10.0f, m_height - 5.0f); }
	void ZoomOut() override { m_height = min(200.0f, m_height + 5.0f); }

private:
	float m_height;  // 고도 (Y축)
	float m_distance; // 확대/축소 시 사용
};
