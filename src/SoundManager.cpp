#include "SoundManager.h"

SoundManager::SoundManager()
{
    sound_engine = createIrrKlangDevice(ESOD_ALSA, ESEO_DEFAULT_OPTIONS);
}

SoundManager::~SoundManager()
{
    sound_engine->drop();
    sound_engine = nullptr;
}

void SoundManager::playClickSound()
{
    irrklang::ISound *sound = sound_engine->play2D("./assets/sounds/Click.wav", false, false, true, ESM_AUTO_DETECT, false);
    sound->setVolume(0.45f);
}

void SoundManager::playSuccessSound()
{
    irrklang::ISound *sound = sound_engine->play2D("./assets/sounds/Success.wav", false, false, true, ESM_AUTO_DETECT, false);
    sound->setVolume(0.4f);
}

void SoundManager::playResetSound()
{
    irrklang::ISound *sound = sound_engine->play2D("./assets/sounds/Reset.wav", false, false, true, ESM_AUTO_DETECT, false);
    sound->setVolume(0.4f);
}

void SoundManager::playBackgroundMusic()
{
    static bool main_theme = true;
    static ISound *sound;
    
    if (sound)
    {
        sound->stop();
        sound->drop();
    }
    
    if (main_theme)
    {
        sound = sound_engine->play2D("./assets/sounds/Menu.wav", true, false, true, ESM_AUTO_DETECT, false);
        sound->setVolume(0.45f);
        sound_engine->play2D("./assets/sounds/Pop.wav", false, false, true, ESM_AUTO_DETECT, false); 
        main_theme = false;
    }
    else
    {
        sound = sound_engine->play2D("./assets/sounds/Rendering.wav", true, false, true, ESM_AUTO_DETECT, false);
        sound->setVolume(0.8f);
        main_theme = true;
    }
}