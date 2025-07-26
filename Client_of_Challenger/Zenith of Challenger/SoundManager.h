#pragma once
#include "stdafx.h"
#include <fmod.hpp>

class SoundManager
{
public:
    bool Initialize();
    void Update();
    void Release();

    void PlaySoundEffect(const std::string& path, bool loop = false);
    void PlayBGM(const std::string& path, bool loop = true);
    void StopAllSounds();
    void StopBGM();

    void SetMute(bool mute);
    bool IsMuted() const { return m_isMuted; }

    void SetMasterVolume(float volume);
    float GetMasterVolume() const;

    void SetBGMVolume(float volume);
    float GetBGMVolume() const;

    void SetSFXVolume(float volume);
    float GetSFXVolume() const;

private:
    FMOD::System* m_pSystem = nullptr;
    FMOD::Channel* m_bgmChannel = nullptr;
    std::unordered_map<std::string, FMOD::Sound*> m_loadedSounds;
    std::vector<FMOD::Channel*> m_channels;

    bool m_isMuted = false;
    float m_masterVolume = 1.0f;
    float m_bgmVolume = 1.0f;
    float m_sfxVolume = 1.0f;
};

extern SoundManager g_Sound;