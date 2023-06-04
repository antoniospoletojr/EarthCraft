#include "SoundManager.h"

SoundManager::SoundManager()
{
    sound_engine = irrklang::createIrrKlangDevice();
}

SoundManager::~SoundManager()
{
    sound_engine->drop();
}

void SoundManager::playClickSound()
{
    irrklang::ISound *sound = sound_engine->play2D("./assets/sounds/Click.wav", false, false, true, irrklang::ESM_AUTO_DETECT, false);
    sound->setVolume(0.15f);
}

void SoundManager::playSuccessSound()
{
    irrklang::ISound *sound = sound_engine->play2D("./assets/sounds/Success.wav", false, false, true, irrklang::ESM_AUTO_DETECT, false);
    sound->setVolume(0.1f);
}

void SoundManager::playResetSound()
{
    irrklang::ISound *sound = sound_engine->play2D("./assets/sounds/Reset.wav", false, false, true, irrklang::ESM_AUTO_DETECT, false);
    sound->setVolume(0.1f);
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
        sound->setVolume(0.05f);
        sound_engine->play2D("./assets/sounds/Pop.wav", false, false, true, ESM_AUTO_DETECT, false); 
        main_theme = false;
    }
    else
    {
        sound = sound_engine->play2D("./assets/sounds/Rendering.wav", true, false, true, ESM_AUTO_DETECT, false);
        sound->setVolume(0.3f);
        main_theme = true;
    }
}