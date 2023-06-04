#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <irrklang/irrKlang.h>

using namespace irrklang;

class SoundManager
{
public:
    SoundManager();
    ~SoundManager();

    void playClickSound();
    void playSuccessSound();
    void playResetSound();
    void playBackgroundMusic();

private:
    irrklang::ISoundEngine *sound_engine;
};

#endif
