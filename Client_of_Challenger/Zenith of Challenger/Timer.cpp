#include "stdafx.h"
#include "Timer.h"

CGameTimer::CGameTimer()
{
    ::QueryPerformanceFrequency((LARGE_INTEGER*)&m_nPerformanceFrequencyPerSec);
    ::QueryPerformanceCounter((LARGE_INTEGER*)&m_nLastPerformanceCounter);
    m_fTimeScale = 1.0 / (double)m_nPerformanceFrequencyPerSec;

    m_nBasePerformanceCounter = m_nLastPerformanceCounter;
    m_nPausedPerformanceCounter = 0;
    m_nStopPerformanceCounter = 0;

    m_nSampleCount = 0;
    m_nCurrentFrameRate = 0;
    m_nFramesPerSecond = 0;
    m_fFPSTimeElapsed = 0.0f;
}

void CGameTimer::Tick(float fLockFPS)
{
    if (m_bStopped)
    {
        m_fTimeElapsed = 0.0f;
        return;
    }

    float fTimeElapsed;
    QueryPerformanceCounter((LARGE_INTEGER*)&m_nCurrentPerformanceCounter);
    fTimeElapsed = float((m_nCurrentPerformanceCounter - m_nLastPerformanceCounter) * m_fTimeScale);

    // FPS 제한 적용 (기본 60FPS 제한)
    if (fLockFPS <= 0.0f) fLockFPS = 60.0f;  // 최소 60FPS 제한

    while (fTimeElapsed < (1.0f / fLockFPS))
    {
        QueryPerformanceCounter((LARGE_INTEGER*)&m_nCurrentPerformanceCounter);
        fTimeElapsed = float((m_nCurrentPerformanceCounter - m_nLastPerformanceCounter) * m_fTimeScale);
    }

    // timeElapsed 값이 갑자기 튀는 것을 방지 (최소/최대값 적용)
    const float minElapsed = 1.0f / 200.0f;  // 최소 200FPS (0.005s)
    const float maxElapsed = 1.0f / 30.0f;   // 최대 30FPS (0.033s)
    fTimeElapsed = max(min(fTimeElapsed, maxElapsed), minElapsed);

    m_nLastPerformanceCounter = m_nCurrentPerformanceCounter;
    m_fTimeElapsed = fTimeElapsed;
}



unsigned long CGameTimer::GetFrameRate(LPTSTR lpszString, int nCharacters)
{
    if (lpszString)
    {
        _itow_s(m_nCurrentFrameRate, lpszString, nCharacters, 10);
        wcscat_s(lpszString, nCharacters, _T(" FPS"));
    }
    return (m_nCurrentFrameRate);
}

float CGameTimer::GetElapsedTime() const
{
    return (m_fTimeElapsed);
}

float CGameTimer::GetTotalTime()
{
    if (m_bStopped) return(float(((m_nStopPerformanceCounter - m_nPausedPerformanceCounter) - m_nBasePerformanceCounter) * m_fTimeScale));
    return(float(((m_nCurrentPerformanceCounter - m_nPausedPerformanceCounter) - m_nBasePerformanceCounter) * m_fTimeScale));
}

UINT CGameTimer::GetFPS() const
{
    return m_nCurrentFrameRate;
}

void CGameTimer::Reset()
{
    __int64 nPerformanceCounter;
    ::QueryPerformanceCounter((LARGE_INTEGER*)&nPerformanceCounter);

    m_nBasePerformanceCounter = nPerformanceCounter;
    m_nLastPerformanceCounter = nPerformanceCounter;
    m_nStopPerformanceCounter = 0;
    m_bStopped = false;
}

void CGameTimer::Start()
{
    __int64 nPerformanceCounter;
    ::QueryPerformanceCounter((LARGE_INTEGER*)&nPerformanceCounter);
    if (m_bStopped)
    {
        m_nPausedPerformanceCounter += (nPerformanceCounter - m_nStopPerformanceCounter);
        m_nLastPerformanceCounter = nPerformanceCounter;
        m_nStopPerformanceCounter = 0;
        m_bStopped = false;
    }
}

void CGameTimer::Stop()
{
    if (!m_bStopped)
    {
        ::QueryPerformanceCounter((LARGE_INTEGER*)&m_nStopPerformanceCounter);
        m_bStopped = true;
    }
}
