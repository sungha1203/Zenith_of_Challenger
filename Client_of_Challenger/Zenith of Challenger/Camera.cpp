#include "camera.h"

Camera::Camera(const ComPtr<ID3D12Device>& device) : m_eye{ 0.f, 0.f, 0.f }, m_at{ 0.f, 0.f, 1.f }, m_up{ 0.f, 1.f, 0.f },
m_u{ 1.f, 0.f, 0.f }, m_v{ 0.f, 1.f, 0.f }, m_n{ 0.f, 0.f, 1.f }
{
	XMStoreFloat4x4(&m_viewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_projectionMatrix, XMMatrixIdentity());
	m_constantBuffer = make_unique<UploadBuffer<CameraData>>(device, (UINT)RootParameter::Camera, true);
}

void Camera::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	XMStoreFloat4x4(&m_viewMatrix,
		XMMatrixLookAtLH(XMLoadFloat3(&m_eye), XMLoadFloat3(&m_at), XMLoadFloat3(&m_up)));

	CameraData buffer;
	XMStoreFloat4x4(&buffer.viewMatrix,
		XMMatrixTranspose(XMLoadFloat4x4(&m_viewMatrix)));
	XMStoreFloat4x4(&buffer.projectionMatrix,
		XMMatrixTranspose(XMLoadFloat4x4(&m_projectionMatrix)));
	buffer.eye = m_eye;
	m_constantBuffer->Copy(buffer);

	m_constantBuffer->UpdateRootConstantBuffer(commandList);
}

void Camera::SetLens(FLOAT fovy, FLOAT aspect, FLOAT minZ, FLOAT maxZ)
{
	XMStoreFloat4x4(&m_projectionMatrix, XMMatrixPerspectiveFovLH(fovy, aspect, minZ, maxZ));
}

void Camera::SetPosition(const XMFLOAT3& position)
{
	m_eye = position;
	UpdateBasis();
}

void Camera::SetLookAt(const XMFLOAT3& lookAt)
{
	m_at = lookAt;
	UpdateBasis();
}

XMFLOAT3 Camera::GetEye() const
{
	return m_eye;
}

XMFLOAT3 Camera::GetU() const
{
	return m_u;
}

XMFLOAT3 Camera::GetV() const
{
	return m_v;
}

XMFLOAT3 Camera::GetN() const
{
	return m_n;
}

void Camera::UpdateBasis()
{
	m_n = Vector3::Normalize(Vector3::Subtract(m_at, m_eye));
	m_u = Vector3::Normalize(Vector3::CrossProduct(m_up, m_n));
	m_v = Vector3::Normalize(Vector3::CrossProduct(m_n, m_u));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ThirdPersonCamera::ThirdPersonCamera(const ComPtr<ID3D12Device>& device) : Camera(device),
m_radius{ Settings::DefaultCameraRadius },
m_phi{ Settings::DefaultCameraPitch }, m_theta{ Settings::DefaultCameraYaw }
{

}

void ThirdPersonCamera::Update(FLOAT timeElapsed)
{

}

void ThirdPersonCamera::UpdateEye(XMFLOAT3 position)
{
	XMFLOAT3 offset{
	m_radius * sin(m_phi) * cos(m_theta),
	m_radius * cos(m_phi),
	m_radius * sin(m_phi) * sin(m_theta) };

	m_eye = Vector3::Add(position, offset);
	m_at = position;
	UpdateBasis();
}

void ThirdPersonCamera::RotatePitch(FLOAT radian)
{
	m_phi += radian;
	m_phi = clamp(m_phi, Settings::CameraMinPitch, Settings::CameraMaxPitch);
}

void ThirdPersonCamera::RotateYaw(FLOAT radian)
{
	m_theta += radian;
}

void ThirdPersonCamera::ZoomIn()
{
	m_radius = max(5.0f, m_radius - 2.0f);
}

void ThirdPersonCamera::ZoomOut()
{
	m_radius = min(30.0f, m_radius + 2.0f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QuarterViewCamera::QuarterViewCamera(const ComPtr<ID3D12Device>& device)
	: Camera(device), m_radius(50.0f), m_phi(XM_PIDIV4), m_theta(-45.0f) //m_phi(XM_PIDIV4)
{
	m_offset = XMFLOAT3(-18.0f, 20.0f, -18.0f);  // 대각선 오프셋 적용
}

void QuarterViewCamera::Update(FLOAT timeElapsed)
{
	// 쿼터뷰는 고정된 각도 유지
}

void QuarterViewCamera::UpdateEye(XMFLOAT3 position)
{
	// m_radius 값을 반영하여 카메라 위치 업데이트
	XMFLOAT3 offset{
		m_radius * cos(m_theta),   // X 방향 이동
		m_radius * sin(m_phi),     // 높이 유지
		m_radius * sin(m_theta)    // Z 방향 이동 (대각선)
	};

	m_eye.x = position.x + offset.x;
	m_eye.y = position.y + offset.y;
	m_eye.z = position.z + offset.z;

	m_at = position;  // 카메라는 항상 플레이어를 바라봄
	UpdateBasis();    // 방향 벡터 업데이트
}

void QuarterViewCamera::RotatePitch(FLOAT radian)
{
	// 쿼터뷰 카메라는 Pitch 회전이 필요 없음
}

void QuarterViewCamera::RotateYaw(FLOAT radian)
{
	// Yaw 회전도 필요하지 않으므로 비워둠
}

void QuarterViewCamera::ZoomIn()
{
	m_radius = max(10.0f, m_radius - 2.0f);  // 최소 거리 제한
	UpdateEye(m_at);  // 줌 변경 후 카메라 업데이트
}

void QuarterViewCamera::ZoomOut()
{
	m_radius = min(40.0f, m_radius + 2.0f);  // 최대 거리 제한
	UpdateEye(m_at);  // 줌 변경 후 카메라 업데이트
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
