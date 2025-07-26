// SoundManager.cpp
#include "SoundManager.h"

SoundManager g_Sound;

bool SoundManager::Initialize()
{
    FMOD::System_Create(&m_pSystem);
    m_pSystem->init(512, FMOD_INIT_NORMAL, nullptr);
    return m_pSystem != nullptr;
}

void SoundManager::Update()
{
    if (m_pSystem) m_pSystem->update();
}

void SoundManager::Release()
{
    for (auto& [path, sound] : m_loadedSounds)
    {
        if (sound) sound->release();
    }
    m_loadedSounds.clear();

    StopAllSounds();

    if (m_pSystem)
    {
        m_pSystem->close();
        m_pSystem->release();
        m_pSystem = nullptr;
    }
}

void SoundManager::PlaySoundEffect(const std::string& path, bool loop)
{
    if (!m_pSystem) return;

    FMOD::Sound* pSound = nullptr;

    if (m_loadedSounds.find(path) != m_loadedSounds.end())
    {
        pSound = m_loadedSounds[path];
    }
    else
    {
        FMOD_MODE mode = FMOD_DEFAULT;
        if (loop) mode |= FMOD_LOOP_NORMAL;

        m_pSystem->createSound(path.c_str(), mode, nullptr, &pSound);
        if (pSound)
        {
            m_loadedSounds[path] = pSound;
        }
    }

    if (pSound)
    {
        FMOD::Channel* pChannel = nullptr;
        m_pSystem->playSound(pSound, nullptr, false, &pChannel);

        if (pChannel)
        {
            pChannel->setVolume(m_isMuted ? 0.0f : m_sfxVolume * m_masterVolume);
            m_channels.push_back(pChannel);
        }
    }
}

void SoundManager::PlayBGM(const std::string& path, bool loop)
{
    if (!m_pSystem) return;

    FMOD::Sound* pSound = nullptr;

    if (m_loadedSounds.find(path) != m_loadedSounds.end())
    {
        pSound = m_loadedSounds[path];
    }
    else
    {
        FMOD_MODE mode = FMOD_DEFAULT;
        if (loop) mode |= FMOD_LOOP_NORMAL;

        m_pSystem->createSound(path.c_str(), mode, nullptr, &pSound);
        if (pSound)
        {
            m_loadedSounds[path] = pSound;
        }
    }

    if (pSound)
    {
        if (m_bgmChannel) m_bgmChannel->stop(); // 이전 BGM 정지
        m_pSystem->playSound(pSound, nullptr, false, &m_bgmChannel);

        if (m_bgmChannel)
        {
            m_bgmChannel->setVolume(m_isMuted ? 0.0f : m_bgmVolume * m_masterVolume);
        }
    }
}

void SoundManager::StopAllSounds()
{
    for (auto& ch : m_channels)
    {
        if (ch) ch->stop();
    }
    m_channels.clear();

    if (m_bgmChannel) m_bgmChannel->stop();
    m_bgmChannel = nullptr;
}

void SoundManager::StopBGM()
{
    if (m_bgmChannel)
    {
        m_bgmChannel->stop();
        m_bgmChannel = nullptr;
    }
}

void SoundManager::SetMute(bool mute)
{
    m_isMuted = mute;

    for (auto& ch : m_channels)
    {
        if (ch) ch->setVolume(mute ? 0.0f : m_sfxVolume * m_masterVolume);
    }

    if (m_bgmChannel)
    {
        m_bgmChannel->setVolume(mute ? 0.0f : m_bgmVolume * m_masterVolume);
    }
}

void SoundManager::SetMasterVolume(float volume)
{
    m_masterVolume = std::clamp(volume, 0.0f, 1.0f);
    SetMute(m_isMuted); // 볼륨 갱신
}

float SoundManager::GetMasterVolume() const { return m_masterVolume; }

void SoundManager::SetBGMVolume(float volume)
{
    m_bgmVolume = std::clamp(volume, 0.0f, 1.0f);
    if (m_bgmChannel && !m_isMuted)
    {
        m_bgmChannel->setVolume(m_bgmVolume * m_masterVolume);
    }
}

float SoundManager::GetBGMVolume() const { return m_bgmVolume; }

void SoundManager::SetSFXVolume(float volume)
{
    m_sfxVolume = std::clamp(volume, 0.0f, 1.0f);
    for (auto& ch : m_channels)
    {
        if (ch && !m_isMuted) ch->setVolume(m_sfxVolume * m_masterVolume);
    }
}

float SoundManager::GetSFXVolume() const { return m_sfxVolume; }
