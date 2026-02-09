#pragma once
#include "Main.h" // For Language enum and GameStrings
#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <iostream>

#define TEXT_COLOR                  255, 255, 255, 255          // White
#define TITLE_BACKGROUND_COLOR      34, 139, 34, 255            // Green
#define BLINK_INTERVAL_MS           500                         // 0.5 seconds

#define TITLE_ANIMATION_SPEED			2.0f					// Velocidade da rotação
#define TITLE_PIECE_SIZE				(WINDOW_HEIGHT * 0.15f) // 15% da altura da janela
#define TITLE_PIECE_SPACING				(WINDOW_HEIGHT * 0.03f) // 3% da altura da janela		
#define TITLE_ANIMATION_STEP_DURATION	1000					// Duração de cada passo em ms
#define TITLE_FULL_CYCLE_DURATION		5000					// Duração total do ciclo em ms

// Screen state
enum class GameState {
	TITLE_SCREEN,
	GAME_SCREEN
};

// Extended GameStrings structure to include title screen strings
struct TitleStrings {
	const char* pressToStart;
	const char* languageOption;
	const char* pressToQuit;
	const char* twoPlayersMode;
	const char* vsAIMode;
};

// Language-specific title strings
static const TitleStrings ENGLISH_TITLE_STRINGS = {
	"PRESS [SPACE] TO START",
	"[L]Choose Language",
	"[Q/ESC]Quit",
	"[P]Game Mode: 2 Players",
	"[P]Game Mode: Vs AI"     
};

static const TitleStrings JAPANESE_TITLE_STRINGS = {
	u8"[スペース]キーを押して開始",
	u8"[L]言語を変更",
	u8"[Q/ESC]終了",
	u8"[P]ゲームモード: 2人",
	u8"[P]ゲームモード: VS AI"
};

static const TitleStrings PORTUGUESE_TITLE_STRINGS = {
	u8"APERTE [ESPAÇO] PARA INICIAR",
	u8"[L]Mudar o Idioma",
	u8"[Q/ESC]Sair",
	u8"[P]Modo de Jogo: 2 Jogadores",
	u8"[P]Modo de Jogo: Vs AI"
};


// Function to get title strings based on language
inline const TitleStrings& GetTitleStrings(Language lang) {
	switch (lang) {
	case Language::Japanese:
		return JAPANESE_TITLE_STRINGS;
	case Language::English:
		return ENGLISH_TITLE_STRINGS;
	case Language::Portuguese:
		return PORTUGUESE_TITLE_STRINGS;
	default:                 // Fallback to English
		return ENGLISH_TITLE_STRINGS;
	}
}

// Utility functions for dynamic font sizing
inline int GetDynamicFontSize(float percentage, int minSize = 12) {
	int calculatedSize = static_cast<int>(WINDOW_HEIGHT * percentage);
	return std::max(calculatedSize, minSize);
}

inline int GetTitleFontSize() {
	return GetDynamicFontSize(0.08f, 24); // 8% of window height, minimum 24px
}

inline int GetRegularFontSize() {
	return GetDynamicFontSize(0.035f, 16); // 3.5% of window height, minimum 16px
}

inline int GetSmallFontSize() {
	return GetDynamicFontSize(0.025f, 12); // 2.5% of window height, minimum 12px
}

// Shared rendering function declarations
void RenderTextWithSize(SDL_Renderer* renderer, const char* text, int x, int y, int fontSize);
void RenderTitleScreen(SDL_Renderer* renderer, TTF_Font* font, Language currentLanguage, Uint32 currentTime, SDL_Texture* pieceTexture);
void HandleTitleScreenEvents(SDL_Event& event, GameState& currentState, bool& quit, Language& currentLanguage, SDL_Window* window);