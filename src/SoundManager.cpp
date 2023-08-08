#include "SoundManager.h"

SoundManager::SoundManager()
{
    // Create an OpenAL device
    device = alcOpenDevice(nullptr);
    if (!device)
    {
        std::cerr << "Failed to open audio device" << std::endl;
        return;
    }
    
    // Create an OpenAL context
    context = alcCreateContext(device, nullptr);
    if (!context)
    {
        std::cerr << "Failed to create audio context" << std::endl;
        return;
    }
    
    // Make the context current
    if (!alcMakeContextCurrent(context))
    {
        std::cerr << "Failed to make audio context current" << std::endl;
        return;
    }
    
    const ALCchar* device_name = nullptr;
	if (alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT"))
		device_name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
	if (!device_name || alcGetError(device) != AL_NO_ERROR)
		device_name = alcGetString(device, ALC_DEVICE_SPECIFIER);
	printf(COLOR_CYAN "Opened \"%s\"\n" COLOR_RESET, device_name);
    
	// Initialize sources
	this->sources.resize(3);
	
	alGenSources(1, &sources[MUSIC]);
	alSourcef(sources[MUSIC], AL_PITCH, 1.f);
	alSourcef(sources[MUSIC], AL_GAIN, 1.f);
	alSource3f(sources[MUSIC], AL_POSITION, 0.f, 0.f, 0.f);
	alSource3f(sources[MUSIC], AL_VELOCITY, 0.f, 0.f, 0.f);
	alSourcei(sources[MUSIC], AL_LOOPING, true);
	
	alGenSources(1, &sources[UI]);
	alSourcef(sources[UI], AL_PITCH, 1.f);
	alSourcef(sources[UI], AL_GAIN, 1.f);
	alSource3f(sources[UI], AL_POSITION, 0.f, 0.f, 0.f);
	alSource3f(sources[UI], AL_VELOCITY, 0.f, 0.f, 0.f);
	alSourcei(sources[UI], AL_LOOPING, false);

	alGenSources(1, &sources[EFFECTS]);
	alSourcef(sources[EFFECTS], AL_PITCH, 1.f);
	alSourcef(sources[EFFECTS], AL_GAIN, 1.f);
	alSource3f(sources[EFFECTS], AL_POSITION, 0.f, 0.f, 0.f);
	alSource3f(sources[EFFECTS], AL_VELOCITY, 0.f, 0.f, 0.f);
	alSourcei(sources[EFFECTS], AL_LOOPING, true);
	alSourcef(sources[EFFECTS], AL_ROLLOFF_FACTOR, 5.f);
	alSourcef(sources[EFFECTS], AL_REFERENCE_DISTANCE, 1000.f);
    
    // Initialize buffers
	loadSound("./assets/sounds/Menu.wav");
	loadSound("./assets/sounds/World.wav");
	loadSound("./assets/sounds/Click.wav");
	loadSound("./assets/sounds/Pop.wav");
	loadSound("./assets/sounds/Reset.wav");
	loadSound("./assets/sounds/Success.wav");
	loadSound("./assets/sounds/Water.wav");
	
	// Set listener position (x, y, z)
	ALfloat listener_position[] = { 0.0f, 0.0f, 0.0f };
	alListenerfv(AL_POSITION, listener_position);
	
	// Set listener velocity (x, y, z)
	ALfloat listener_velocity[] = { 0.0f, 0.0f, 0.0f };
	alListenerfv(AL_VELOCITY, listener_velocity);
	
	// Set listener orientation (forward direction, up direction)
	ALfloat listener_orientation[] = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
	alListenerfv(AL_ORIENTATION, listener_orientation);

	alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
}

SoundManager::~SoundManager()
{
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);

    alDeleteBuffers(buffers.size(), buffers.data());
	buffers.clear();
	
	alDeleteSources(1, &sources[MUSIC]);
	alDeleteSources(1, &sources[UI]);
}

void SoundManager::loadSound(const char* filename)
{
	ALenum err, format;
	ALuint buffer;
	SNDFILE* sndfile;
	SF_INFO sfinfo;
	short* membuf;
	sf_count_t num_frames;
	ALsizei num_bytes;
	
	// Open the audio file and check that it's usable
	sndfile = sf_open(filename, SFM_READ, &sfinfo);
	if (!sndfile)
	{
		fprintf(stderr, "Could not open audio in %s: %s\n", filename, sf_strerror(sndfile));
		return;
	}
	if (sfinfo.frames < 1 || sfinfo.frames >(sf_count_t)(INT_MAX / sizeof(short)) / sfinfo.channels)
	{
		fprintf(stderr, "Bad sample count in %s (%" PRId64 ")\n", filename, sfinfo.frames);
		sf_close(sndfile);
		return;
	}
	
	// Get the sound format, and figure out the OpenAL format
	format = AL_NONE;
	if (sfinfo.channels == 1)
		format = AL_FORMAT_MONO16;
	else if (sfinfo.channels == 2)
		format = AL_FORMAT_STEREO16;
	else if (sfinfo.channels == 3)
	{
		if (sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
			format = AL_FORMAT_BFORMAT2D_16;
	}
	else if (sfinfo.channels == 4)
	{
		if (sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
			format = AL_FORMAT_BFORMAT3D_16;
	}
	if (!format)
	{
		fprintf(stderr, "Unsupported channel count: %d\n", sfinfo.channels);
		sf_close(sndfile);
		return;
	}

	// Decode the whole audio file to a buffer
	membuf = static_cast<short*>(malloc((size_t)(sfinfo.frames * sfinfo.channels) * sizeof(short)));

	num_frames = sf_readf_short(sndfile, membuf, sfinfo.frames);
	if (num_frames < 1)
	{
		free(membuf);
		sf_close(sndfile);
		fprintf(stderr, "Failed to read samples in %s (%" PRId64 ")\n", filename, num_frames);
		return;
	}
	num_bytes = (ALsizei)(num_frames * sfinfo.channels) * (ALsizei)sizeof(short);
	
	//Buffer the audio data into a new buffer object, then free the data and close the file
	alGenBuffers(1, &buffer);
	alBufferData(buffer, format, membuf, num_bytes, sfinfo.samplerate);

	free(membuf);
	sf_close(sndfile);
	
	// Check if an error occured, and clean up if so
	err = alGetError();
	if (err != AL_NO_ERROR)
	{
		fprintf(stderr, "OpenAL Error: %s\n", alGetString(err));
		if (buffer && alIsBuffer(buffer))
			alDeleteBuffers(1, &buffer);
		return;
	}
	
	this->buffers.push_back(buffer);  // add to the list of known buffers
}

void SoundManager::playClickSound()
{
	alSourceStop(sources[UI]);
	alSourcei(sources[UI], AL_BUFFER, (ALint)buffers[CLICK_SOUND]);
	alSourcePlay(sources[UI]); 
}

void SoundManager::playSuccessSound()
{
	alSourceStop(sources[UI]);
	alSourcei(sources[UI], AL_BUFFER, (ALint)buffers[SUCCESS_SOUND]);
	alSourcePlay(sources[UI]); 
}

void SoundManager::playResetSound()
{
	alSourceStop(sources[EFFECTS]);
	alSourceStop(sources[UI]);
	alSourcei(sources[UI], AL_BUFFER, (ALint)buffers[RESET_SOUND]);
	alSourcePlay(sources[UI]); 
}

void SoundManager::playBackgroundMusic()
{	
    static bool main_theme = true;
    
    if (main_theme)
    {
		alSourceStop(sources[MUSIC]);
		alSourcei(sources[MUSIC], AL_BUFFER, (ALint)buffers[MENU_SOUND]);
		alSourcePlay(sources[MUSIC]);
		
		alSourceStop(sources[UI]);
		alSourcei(sources[UI], AL_BUFFER, (ALint)buffers[POP_SOUND]);
		alSourcePlay(sources[UI]);
        main_theme = false;
    }
    else
    {
		alSourceStop(sources[MUSIC]);
		alSourcei(sources[MUSIC], AL_BUFFER, (ALint)buffers[WORLD_SOUND]);
		alSourcePlay(sources[MUSIC]);
		
		alSourceStop(sources[EFFECTS]);
		alSourcei(sources[EFFECTS], AL_BUFFER, (ALint)buffers[WATER_SOUND]);
		alSourcePlay(sources[EFFECTS]);
        main_theme = true;
    }
}

void SoundManager::updateListener(float distance)
{
	// Set listener position (x, y, z)
	printf("Distance: %f\n", distance);
	alSource3f(sources[EFFECTS], AL_POSITION, 0.f, distance, 0.f);
}