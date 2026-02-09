#pragma once
#include <SDL_mixer.h>
#include <string>
#include <map>
#include <vector>

class SoundSystem {

public:
    enum SoundType {
        EAT_FOOD,
        GAME_OVER,
        MENU_SELECT
    };

    // Predefined sound file names
    static constexpr const char* SOUND_FILES[] = {
        "sounds/eat.mp3",       // EAT_FOOD
        "sounds/gameover.mp3",  // GAME_OVER
        "sounds/select.mp3"     // MENU_SELECT
    };

    // Adicione o caminho do arquivo de música
    static constexpr const char* BGM_FILES[] = {
        "sounds/BGM1.mp3",
        "sounds/BGM2.mp3",
        "sounds/BGM3.mp3"
    };

    static bool Initialize();
    static void Shutdown();
    static void LoadSounds();
    static void PlaySound(SoundType type);
    static void SetVolume(int volume); // 0-128
    static void ToggleMute();

    // Adição para o BGM
    static void PlayBgm(int index); // Novo: Toca o BGM pelo índice
    static void StopBgm();
    static void SetBgmVolume(int volume);
    static void ToggleBgmMute();

    // Variáveis de controle de volume
    static int currentVolume;
    static int bgmVolume;
    static bool isMuted;
    static bool isBgmMuted;

private:
    static std::map<SoundType, Mix_Chunk*> sounds;
    static std::vector<Mix_Music*> bgms;
    static bool isInitialized;
    static Mix_Music* currentBgm;
};