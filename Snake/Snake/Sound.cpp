#include "Sound.h"
#include <iostream>
#include <algorithm>
#include <SDL_mixer.h>

std::map<SoundSystem::SoundType, Mix_Chunk*> SoundSystem::sounds;
std::vector<Mix_Music*> SoundSystem::bgms; // Novo: para armazenar múltiplos BGMs
bool SoundSystem::isInitialized = false;
bool SoundSystem::isMuted = false;
int SoundSystem::currentVolume = MIX_MAX_VOLUME / 2; // Default to half volume

// Variáveis e ponteiros para o BGM
int SoundSystem::bgmVolume = MIX_MAX_VOLUME / 2;
bool SoundSystem::isBgmMuted = false;
Mix_Music* SoundSystem::currentBgm = nullptr;

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

    StopBgm();

    // Libera a memória de todos os BGMs carregados
    for (auto& bgm : bgms) {
        Mix_FreeMusic(bgm);
    }
    bgms.clear();

    for (auto& sound : sounds) {
        Mix_FreeChunk(sound.second);
    }
    sounds.clear();

    Mix_CloseAudio();
    isInitialized = false;
}

void SoundSystem::LoadSounds() {
    if (!isInitialized) return;

    // Carrega os efeitos sonoros do array SOUND_FILES
    for (int i = 0; i < std::size(SOUND_FILES); ++i) {
        Mix_Chunk* sound = Mix_LoadWAV(SOUND_FILES[i]);
        if (!sound) {
            std::cerr << "Failed to load sound: " << SOUND_FILES[i]
                << " Error: " << Mix_GetError() << std::endl;
        }
        else {
            // Usa o índice 'i' para mapear o som carregado para o seu tipo
            sounds[static_cast<SoundType>(i)] = sound;
        }
    }

    // Carrega todos os BGMs do array
    for (int i = 0; i < std::size(BGM_FILES); i++) {
        Mix_Music* bgm = Mix_LoadMUS(BGM_FILES[i]);
        if (!bgm) {
            std::cerr << "Failed to load BGM: " << BGM_FILES[i]
                << " Error: " << Mix_GetError() << std::endl;
        }
        else {
            bgms.push_back(bgm);
        }
    }
}

void SoundSystem::PlaySound(SoundType type) {
    if (!isInitialized || isMuted) return;

    auto it = sounds.find(type);
    if (it != sounds.end()) {
        Mix_PlayChannel(-1, it->second, 0);
    }
    else {
        std::cerr << "Sound not found for type: " << static_cast<int>(type) << std::endl;
    }
}

// Aumenta ou diminui o volume dos efeitos sonoros
void SoundSystem::SetVolume(int volume) {
    if (!isInitialized) return;
    currentVolume = std::max(0, std::min(volume, MIX_MAX_VOLUME));
    Mix_Volume(-1, currentVolume);
}

void SoundSystem::ToggleMute() {
    isMuted = !isMuted;
    if (isMuted) {
        Mix_Volume(-1, 0);
    }
    else {
        Mix_Volume(-1, currentVolume);
    }
}

// --- Funções para o BGM ---
void SoundSystem::PlayBgm(int index) {
    if (!isInitialized || bgms.empty() || index < 0 || index >= bgms.size()) {
        std::cerr << "Invalid BGM index or no BGMs loaded." << std::endl;
        return;
    }

    if (Mix_PlayingMusic()) {
        Mix_HaltMusic(); // Para a música atual antes de tocar a nova
    }

    currentBgm = bgms[index];
    Mix_PlayMusic(currentBgm, -1); // Toca em loop infinito (-1)
    Mix_VolumeMusic(isBgmMuted ? 0 : bgmVolume);
}

void SoundSystem::StopBgm() {
    if (Mix_PlayingMusic()) {
        Mix_HaltMusic();
    }
}

void SoundSystem::SetBgmVolume(int volume) {
    if (!isInitialized) return;

    // Armazena o novo volume, mas não o aplica se estiver mutado
    bgmVolume = std::max(0, std::min(volume, MIX_MAX_VOLUME));

    if (!isBgmMuted) {
        Mix_VolumeMusic(bgmVolume);
    }
}

void SoundSystem::ToggleBgmMute() {
    isBgmMuted = !isBgmMuted;
    if (isBgmMuted) {
        Mix_VolumeMusic(0);
    }
    else {
        Mix_VolumeMusic(bgmVolume);
    }
}