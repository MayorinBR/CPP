#pragma once
#include <SDL.h>
#ifdef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#else
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#endif

#define MIN_WINDOW_WIDTH       600			// Minimum window width
#define MIN_WINDOW_HEIGHT      400			// Minimum window height
#define ASPECT_RATIO           4.0f/3.0f	// Target aspect ratio

// Game constants
#define GRID_SIZE               8						// 8x8
#define RESTART_TIME            3       
#define TEXT_SIZE               24
#define TEXT_FONT               "NotoSansJP-Variable.ttf"

#define GAME_BACKGROUND_COLOR   34, 139, 34, 255		// Verde para tabuleiro de Otelo
#define BLACK_COLOR             0, 0, 0, 255			// Preto
#define WHITE_COLOR             255, 255, 255, 255		// Branco
#define GRID_COLOR              0, 100, 0, 255			// Verde mais escuro para linhas
#define HINT_COLOR              255, 255, 0, 150		// Amarelo para jogadas possíveis

#define ANIMATION_DURATION_MS   750					// Duração da animação em milissegundos
#define PI                      3.14159265358979323846f

extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;
extern int CELL_SIZE;
extern int GRID_WIDTH;
extern int GRID_HEIGHT;
extern int GRID_OFFSET_X;
extern int GRID_OFFSET_Y;

extern SDL_Texture* pieceSpriteSheet;
extern SDL_Rect blackPieceRect;
extern SDL_Rect whitePieceRect;

// Estrutura para animação de peça
struct PieceAnimation {
	int row;
	int col;
	char startPiece;
	char endPiece;
	Uint32 startTime;
	bool active;
};

// Language enumeration
enum class Language {
	Japanese,
	English,
	Portuguese,
	LANGUAGE_COUNT
};

// Game Mode enumeration
enum class GameMode {
	TwoPlayers,
	VsAI,
	GAME_MODE_COUNT
};

extern GameMode currentGameMode;

// Language strings structure
struct GameStrings {
	const char* windowName;
	const char* winMessage;
	const char* drawMessage;
	const char* scoreText;
	const char* turnText;
	const char* restartMessage;
	const char* returnTitleMessage;
	const char* noMovesMessage;
	const char* blackMessage;
	const char* whiteMessage;
	const char* gameModeText;
};

// English strings
static const GameStrings ENGLISH_STRINGS = {
	"Reversi",
	"%s Wins!",
	"Draw!",
	"Black: %d | White: %d",
	"%s Player's turn...",
	"Restarting in %d s...",
	"Press T to return to title",
	"No valid moves. Turn passes to opponent.",
	"Black",
	"White",
	"Game Mode: %s (Press P to change)"
};

// Japanese strings
static const GameStrings JAPANESE_STRINGS = {
	u8"オセロ",
	u8"%s の勝利!",
	u8"引き分けです!",
	u8"黒: %d | 白: %d",
	u8"%s の番です...",
	u8"%d秒後に再開します...",
	u8"Tキーでタイトルに戻る",
	u8"有効な手がありません。相手の番になります。",
	u8"黒",
	u8"白",
	u8"ゲームモード: %s (Pキーで変更)"
};

// Portuguese strings
static const GameStrings PORTUGUESE_STRINGS = {
	"Reversi",
	"%s venceu!",
	"Empate!",
	"Pretas: %d | Brancas: %d",
	"Turno do jogador %s...",
	"Recomecando em %d s...",
	"Pressione T para voltar ao titulo",
	"Sem jogadas validas. Turno passa para o oponente.",
	"Pretas",
	"Brancas",
	"Modo de Jogo: %s (Pressione P para mudar)"
};

inline const GameStrings& GetGameStrings(Language lang) {
	switch (lang) {
	case Language::Japanese:
		return JAPANESE_STRINGS;
	case Language::English:
		return ENGLISH_STRINGS;
	case Language::Portuguese:
		return PORTUGUESE_STRINGS;
	default:
		return ENGLISH_STRINGS;
	}
}

void CountPieces(int& black, int& white);
int GetRelativeX(float percentage);
int GetRelativeY(float percentage);
void HandleWindowResize(SDL_Window* window);