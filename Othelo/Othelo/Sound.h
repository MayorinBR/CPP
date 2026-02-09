#pragma once
#include <SDL_mixer.h>
#include <string>
#include <map>

class SoundSystem {

public:
	enum SoundType {

		PIECE_PLACE,
		PIECE_FLIP,
		GAME_OVER,
		MENU_SELECT,
		MENU_CHANGE,
		INVALID_MOVE
	};

	// Predefined sound file names
	static constexpr const char* SOUND_FILES[6] = {
		"sounds/place.mp3",      // PIECE_PLACE
		"sounds/flip.mp3",       // PIECE_FLIP
		"sounds/gameover.mp3",   // GAME_OVER
		"sounds/select.mp3",     // MENU_SELECT
		"sounds/change.mp3",     // MENU_CHANGE
		"sounds/invalid.mp3"     // INVALID_MOVE

	};

	static bool Initialize();
	static void Shutdown();
	static void LoadSounds();
	static void PlaySound(SoundType type);
	static void SetVolume(int volume); // 0-128
	static void ToggleMute();
	static bool IsMuted(); // ADDED THIS LINE

private:
	static std::map<SoundType, Mix_Chunk*> sounds;
	static bool isInitialized;
	static bool isMuted;
	static int currentVolume;
};