#include "SoundManager.h"

SoundManager g_Sound; 

bool SoundManager::Initialize()
{
    FMOD_RESULT result = FMOD::System_Create(&m_pSystem);
    if (result != FMOD_OK) return false;

    result = m_pSystem->init(512, FMOD_INIT_NORMAL, 0);
    return result == FMOD_OK;
}

void SoundManager::Update()
{
    if (m_pSystem) m_pSystem->update();
}

void SoundManager::Release()
{
    if (m_pSystem)
    {
        m_pSystem->close();
        m_pSystem->release();
        m_pSystem = nullptr;
    }
}

void SoundManager::PlaySoundEffect(const std::string& path, bool loop)
{
    FMOD::Sound* pSound = nullptr;
    FMOD_MODE mode = loop ? FMOD_LOOP_NORMAL : FMOD_DEFAULT;

    m_pSystem->createSound(path.c_str(), mode, 0, &pSound);

    if (pSound)
    {
        m_pSystem->playSound(pSound, nullptr, false, nullptr);
    }
}
