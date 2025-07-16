#include "MagicBall.h"
#include "MagicBallTrail.h"
#include "SceneManager.h"
#include "Scene.h"
#include "GameFramework.h"

using namespace DirectX;

MagicBall::MagicBall(const ComPtr<ID3D12Device>& device)
    : GameObject(device)
{
    SetUseTexture(false); // 텍스처 없이 보라색 표현
    SetBaseColor(XMFLOAT4(0.7f, 0.3f, 1.0f, 1.0f));
    SetScale(XMFLOAT3(0.2f, 0.2f, 0.2f));

    m_trailElapsed = 0.0f;
    m_trailInterval = 0.05f; // 트레일 생성 간격 (초)
}

void MagicBall::Update(FLOAT timeElapsed)
{
    m_aliveTime += timeElapsed;
    if (IsDead()) return;

    XMFLOAT3 pos = GetPosition();

    // 기본 이동
    pos.x += m_direction.x * m_speed * timeElapsed;
    pos.y += m_direction.y * m_speed * timeElapsed;
    pos.z += m_direction.z * m_speed * timeElapsed;

    // ---------------------------
    // 평타(기본): 흔들림 추가
    if (m_type == MagicBallType::Normal)
    {
        float waveX = sinf(m_aliveTime * 10.0f + m_waveOffsetX);
        float waveY = cosf(m_aliveTime * 7.0f + m_waveOffsetY);
        float waveAmp = 0.25f;
        pos.x += waveX * waveAmp;
        pos.y += waveY * waveAmp;
    }

    SetPosition(pos);

    // ---------------------------
    // 회전: 평타만 회전
    XMMATRIX rot = XMMatrixIdentity();
    if (m_type == MagicBallType::Normal)
    {
        float yaw = XMConvertToRadians(m_aliveTime * 360.0f);
        rot = XMMatrixRotationY(yaw);
    }

    // ---------------------------
    // 스케일 진동
    float pulseX = 1.0f + m_scaleAmpX * sinf(m_aliveTime * m_scaleFreqX);
    float pulseY = 1.0f + m_scaleAmpY * cosf(m_aliveTime * m_scaleFreqY);
    float pulseZ = 1.0f + m_scaleAmpZ * sinf(m_aliveTime * m_scaleFreqZ);

    XMMATRIX scl = XMMatrixScaling(
        pulseX * m_scale.x,
        pulseY * m_scale.y,
        pulseZ * m_scale.z
    );

    XMMATRIX trn = XMMatrixTranslation(pos.x, pos.y, pos.z);
    SetWorldMatrix(scl * rot * trn);

    // 트레일
    m_trailElapsed += timeElapsed;
    if (m_trailElapsed >= m_trailInterval)
    {
        m_trailElapsed = 0.0f;
        CreateTrail();
    }
}

void MagicBall::CreateTrail()
{
    auto trail = std::make_shared<MagicBallTrail>(gGameFramework->GetDevice());
    trail->SetMesh(m_mesh);
    trail->SetShader(m_shader);
    trail->SetPosition(GetPosition());
    trail->SetWorldMatrix(XMLoadFloat4x4(&GetWorldMatrix()));
    trail->SetBaseColor(XMFLOAT4(1.0f, 0.2f, 1.0f, 1.0f));

    gGameFramework->GetSceneManager()->GetCurrentScene()->AddTrailObject(trail);
}


MagicImpactEffect::MagicImpactEffect(const ComPtr<ID3D12Device>& device)
    : GameObject(device)
{
    m_isActive = true;
}

void MagicImpactEffect::Update(FLOAT timeElapsed)
{
    m_elapsed += timeElapsed;

    if (m_elapsed > m_lifetime)
    {
        m_isActive = false; // 비활성화 처리
        return;
    }

    float scale = 1.5f + 2.0f * sin(m_elapsed * XM_PI * 2.0f); // 파동처럼 터짐
    SetScale(XMFLOAT3(scale, scale, scale));
}
