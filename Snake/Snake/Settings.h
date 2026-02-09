#pragma once
#include <SDL.h>
#include <string>
#include <vector>
#include "Main.h"
#include "Sound.h"

// Define as cores
struct Color {
    int r, g, b, a;
};

// Declara as texturas como extern
extern SDL_Texture* muteIconTexture;
extern SDL_Texture* unmuteIconTexture;


// Protótipos
void RenderSettingsScreen(SDL_Renderer* renderer);
void HandleSettingsScreenEvents(SDL_Event& event, GameState& currentState, bool& quit);
void applyColor(const std::string& key, const Color& color);
void RenderColorRect(SDL_Renderer* renderer, const Color& color, int x, int y);
void RenderVolumeBar(SDL_Renderer* renderer, int x, int y, int volume, int barWidth, int barHeight);
void LoadVolumeIcons(SDL_Renderer* renderer);
void RenderVolumeIcon(SDL_Renderer* renderer, int x, int y, bool isMuted, int volume);
void RenderCheckbox(SDL_Renderer* renderer, int x, int y, bool checked);
void CleanupVolumeIcons();