/**
@file
@brief SoundManager header file.
*/

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

/**
 * @brief SoundManager class which handles the sound effects and background music.
 */
class SoundManager
{
public:
    
    /**
     * @brief Construct a new Sound Manager object
     */
    SoundManager();

    /**
     * @brief Destroy the Sound Manager object
     */
    ~SoundManager();
    
    /**
     * @brief Update the listener's position for 3D positional audio of water and wind sound effects.
     * 
     * @param distance Distance from the camera to the water surface
     */
    void updateListener(float distance);

    /**
     * @brief Set the wind altitude in order to fade the wind sound effect.
     * 
     * @param altitude Altitude of the wind
     */
    void setWindAltitude(float altitude);
    
    /**
     * @brief Play the click sound effect.
     */
    void playClickSound();

    /**
     * @brief Play the success sound effect.
     */
    void playSuccessSound();

    /**
     * @brief Play the reset sound effect.
     */
    void playResetSound();

    /**
     * @brief Play the background musics (splashscreen and rendering)
     */
    void playBackgroundMusic();

private:
    ALCdevice* device;              ///< OpenAL device reference
    ALCcontext* context;            ///< OpenAL context reference
    
    std::vector<ALuint> buffers;    ///< OpenAL buffers vector containing the audios
    std::vector<ALuint> sources;    ///< OpenAL sources vector containing the sources of the audio
    
    /**
     * @brief Load the sound from the wav file.
     * 
     * @param filename File path of the wav file
     */
    void loadSound(const char* filename);
};

#endif
