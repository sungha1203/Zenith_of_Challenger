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

    // FPS 제한이 0보다 클 때만 제한을 적용함
    if (fLockFPS > 0.0f)
    {
        while (fTimeElapsed < (1.0f / fLockFPS))
        {
            QueryPerformanceCounter((LARGE_INTEGER*)&m_nCurrentPerformanceCounter);
            fTimeElapsed = float((m_nCurrentPerformanceCounter - m_nLastPerformanceCounter) * m_fTimeScale);
        }
    }

    const float minElapsed = 1.0f / 200.0f;
    const float maxElapsed = 1.0f / 30.0f;
    fTimeElapsed = max(min(fTimeElapsed, maxElapsed), minElapsed);

    m_nLastPerformanceCounter = m_nCurrentPerformanceCounter;
    m_fTimeElapsed = fTimeElapsed;

    // FPS 계산
    m_fFPSTimeElapsed += fTimeElapsed;
    ++m_nFramesPerSecond;

    if (m_fFPSTimeElapsed >= 1.0f)
    {
        m_nCurrentFrameRate = m_nFramesPerSecond;
        m_nFramesPerSecond = 0;
        m_fFPSTimeElapsed = 0.0f;
    }
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
