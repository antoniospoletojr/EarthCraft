#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include "AL/al.h"
#include <AL/alc.h>
#include <AL/alext.h>
#include <sndfile.h>
#include <inttypes.h>
#include <climits> 
#include <iostream>
#include <vector>
#include "Colors.h"
#include "Constants.h"
#include "Vec.hpp"


class SoundManager
{
public:
    SoundManager();
    ~SoundManager();

    void updateListener(float distance);
    void setWindAltitude(float altitude);
    
    void playClickSound();
    void playSuccessSound();
    void playResetSound();
    void playBackgroundMusic();

private:
    ALCdevice* device;
    ALCcontext* context;
    
    std::vector<ALuint> buffers, sources;

    void loadSound(const char* filename);
};

#endif
