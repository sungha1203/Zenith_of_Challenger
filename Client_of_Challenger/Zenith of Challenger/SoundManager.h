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

private:
    FMOD::System* m_pSystem = nullptr;
};

extern SoundManager g_Sound;
