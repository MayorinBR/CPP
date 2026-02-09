#pragma once
#include "LanguageManager.h"
#include "Sound.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <iostream>
#include <string>

// Colors and constants definitions
#define TEXT_COLOR                  255, 255, 255, 255          // White
#define BLINK_INTERVAL_MS           500                         // 0.5 seconds

// Title Image definitions and texture
#define TITLE_IMAGE_WIDTH           1280
#define TITLE_IMAGE_HEIGHT          580
static constexpr const char* TITLE_IMAGE_FILE = "assets/SnakeTitle.png";

extern SDL_Texture* titleTexture;

// Enumerations
enum class GameState;
enum class Language; 

extern SDL_Color textColor;

// File paths for flag images
static constexpr const char* FLAG_FILES[] = {
    "assets/flags/US-UK.png", // ENGLISH
    "assets/flags/JP.png",    // JAPANESE
    "assets/flags/PT-BR.png"  // PORTUGUESE
};

// Function prototypes
void RenderCenteredText(SDL_Renderer* renderer, const char* text, int y, int fontSize);
void RenderTextAtPosition(SDL_Renderer* renderer, const std::string& text, int x, int y, int fontSize, SDL_Color color);
void RenderTitleScreen(SDL_Renderer* renderer);
void RenderDifficultyScreen(SDL_Renderer* renderer);
void RenderLanguageScreen(SDL_Renderer* renderer);
void RenderHighscoresScreen(SDL_Renderer* renderer);
void RenderNameInputScreen(SDL_Renderer* renderer); 
void HandleTitleScreenEvents(SDL_Event& event, GameState& currentState, bool& quit);
void HandleDifficultyScreenEvents(SDL_Event& event, GameState& currentState);
void HandleLanguageScreenEvents(SDL_Event& event, GameState& currentState, bool& quit);
void HandleHighscoresScreenEvents(SDL_Event& event, GameState& currentState);
void HandleNameInputEvents(SDL_Event& event, GameState& currentState, bool& quit);
void RenderSettingsScreen(SDL_Renderer* renderer);
void HandleSettingsScreenEvents(SDL_Event& event, GameState& currentState, bool& quit);
bool LoadTitleTexture(SDL_Renderer* renderer);
void CleanupTitleTexture();