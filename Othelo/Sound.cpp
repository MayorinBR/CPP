#include "Sound.h"
#include <iostream>

std::map<SoundSystem::SoundType, Mix_Chunk*> SoundSystem::sounds;
bool SoundSystem::isInitialized = false;
bool SoundSystem::isMuted = false;
int SoundSystem::currentVolume = MIX_MAX_VOLUME / 2; // Default to half volume

bool SoundSystem::Initialize() {
	if (isInitialized) return true;

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		std::cerr << "SDL_mixer could not initialize! Error: " << Mix_GetError() << std::endl;
		return false;
	}

	isInitialized = true;
	Mix_Volume(-1, currentVolume); // Set volume for all channels
	return true;
}

void SoundSystem::Shutdown() {
	if (!isInitialized) return;

	for (auto& sound : sounds) {
		Mix_FreeChunk(sound.second);
	}
	sounds.clear();

	Mix_CloseAudio();
	isInitialized = false;
}

void SoundSystem::LoadSounds() {
	if (!isInitialized) return;

	for (int i = 0; i < 6; i++) {
		SoundType type = static_cast<SoundType>(i);
		Mix_Chunk* sound = Mix_LoadWAV(SOUND_FILES[i]);
		if (!sound) {
			std::cerr << "Failed to load sound: " << SOUND_FILES[i]
				<< " Error: " << Mix_GetError() << std::endl;
			continue;
		}
		sounds[type] = sound;
	}
}

void SoundSystem::PlaySound(SoundType type) {
	if (!isInitialized || isMuted) return;

	auto it = sounds.find(type);
	if (it != sounds.end()) {
		Mix_PlayChannel(-1, it->second, 0);
	}
}

void SoundSystem::SetVolume(int volume) {
	currentVolume = volume;
	if (isInitialized) {
		Mix_Volume(-1, currentVolume);
	}
}

void SoundSystem::ToggleMute() {
	isMuted = !isMuted;
	if (isInitialized) {
		Mix_Volume(-1, isMuted ? 0 : currentVolume);
	}
}

bool SoundSystem::IsMuted() {
	return isMuted;
}