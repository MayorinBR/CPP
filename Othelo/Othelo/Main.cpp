#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h> 
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include "Main.h"
#include "Title.h"
#include "Cheats.h"
#include "Sound.h"
#include "AI.h"

using namespace std;

// Screen size variables
int WINDOW_WIDTH = MIN_WINDOW_WIDTH;
int WINDOW_HEIGHT = MIN_WINDOW_HEIGHT;
int CELL_SIZE = 40;
int GRID_WIDTH = CELL_SIZE * GRID_SIZE;
int GRID_HEIGHT = CELL_SIZE * GRID_SIZE;
int GRID_OFFSET_X = (WINDOW_WIDTH - GRID_WIDTH) / 2;
int GRID_OFFSET_Y = (WINDOW_HEIGHT - GRID_HEIGHT) / 2;

// Game assets
SDL_Texture* pieceSpriteSheet = nullptr;
SDL_Rect blackPieceRect = { 0, 0, 300, 300 };
SDL_Rect whitePieceRect = { 300, 0, 300, 300 };

// Event handling
SDL_Event event;
bool quit = false;

// Game state
vector<vector<char>> board(GRID_SIZE, vector<char>(GRID_SIZE, ' '));  // 8x8 grid
char currentPlayer = 'B';  // B (Black) starts first
bool gameOver = false;
int blackScore = 2;
int whiteScore = 2;
vector<pair<int, int>> validMoves;   // Valid moves
bool passTurn = false;               // Flag for turn passing
Uint32 gameOverTime = 0;
Language currentLanguage = Language::Japanese;
GameMode currentGameMode = GameMode::TwoPlayers;

// Animation variables
vector<PieceAnimation> activeAnimations;

int GetRelativeX(float percentage) {
	return static_cast<int>(WINDOW_WIDTH * percentage);
}

int GetRelativeY(float percentage) {
	return static_cast<int>(WINDOW_HEIGHT * percentage);
}

void SetWindowMinMaxSize(SDL_Window* window) {
	SDL_SetWindowMinimumSize(window, MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT);
}

void ResetGame() {
	board = vector<vector<char>>(GRID_SIZE, vector<char>(GRID_SIZE, ' '));
	int center = GRID_SIZE / 2;
	board[center - 1][center - 1] = 'W';
	board[center][center] = 'W';
	board[center - 1][center] = 'B';
	board[center][center - 1] = 'B';

	currentPlayer = 'B';
	gameOver = false;
	CountPieces(blackScore, whiteScore); // Update count
	validMoves.clear();
	passTurn = false;
	activeAnimations.clear();
}

bool IsValidMove(int row, int col, char player) {
	if (board[row][col] != ' ') return false;

	char opponent = (player == 'B') ? 'W' : 'B';
	bool isValid = false;

	// Check all 8 directions
	for (int dr = -1; dr <= 1; dr++) {
		for (int dc = -1; dc <= 1; dc++) {
			if (dr == 0 && dc == 0) continue;

			int r = row + dr;
			int c = col + dc;
			bool foundOpponent = false;

			while (r >= 0 && r < GRID_SIZE && c >= 0 && c < GRID_SIZE) {
				if (board[r][c] == opponent) {
					foundOpponent = true;
				}
				else if (board[r][c] == player && foundOpponent) {
					isValid = true;
					break;
				}
				else {
					break;
				}

				r += dr;
				c += dc;
			}

			if (isValid) break;
		}
	}

	return isValid;
}

void FindValidMoves(char player) {
	validMoves.clear();

	for (int row = 0; row < GRID_SIZE; row++) {
		for (int col = 0; col < GRID_SIZE; col++) {
			if (IsValidMove(row, col, player)) {
				validMoves.emplace_back(row, col);
			}
		}
	}
}

void CountPieces(int& black, int& white) {
	black = 0;
	white = 0;

	for (int row = 0; row < GRID_SIZE; row++) {
		for (int col = 0; col < GRID_SIZE; col++) {
			if (board[row][col] == 'B') black++;
			else if (board[row][col] == 'W') white++;
		}
	}
}

void MakeMove(int row, int col, char player) {
	if (!IsValidMove(row, col, player)) {
		SoundSystem::PlaySound(SoundSystem::INVALID_MOVE);
		return;
	}

	char opponent = (player == 'B') ? 'W' : 'B';
	board[row][col] = player;
	SoundSystem::PlaySound(SoundSystem::PIECE_PLACE);

	// Flip pieces in all directions
	for (int dr = -1; dr <= 1; dr++) {
		for (int dc = -1; dc <= 1; dc++) {
			if (dr == 0 && dc == 0) continue;

			int r = row + dr;
			int c = col + dc;
			vector<pair<int, int>> toFlip;

			while (r >= 0 && r < GRID_SIZE && c >= 0 && c < GRID_SIZE) {
				if (board[r][c] == opponent) {
					toFlip.emplace_back(r, c);
				}
				else if (board[r][c] == player) {
					for (auto& pos : toFlip) {
						PieceAnimation anim;
						anim.row = pos.first;
						anim.col = pos.second;
						anim.startPiece = opponent;
						anim.endPiece = player;
						anim.startTime = SDL_GetTicks();
						anim.active = true;
						activeAnimations.push_back(anim);
					}
					break;
				}
				else {
					break;
				}
				r += dr;
				c += dc;
			}
		}
	}

	// Update count based on actual board
	CountPieces(blackScore, whiteScore);
}

void UpdateGameState() {
	// Update real count
	CountPieces(blackScore, whiteScore);

	// Check valid moves
	bool blackCanMove = false, whiteCanMove = false;
	FindValidMoves('B');
	blackCanMove = !validMoves.empty();
	FindValidMoves('W');
	whiteCanMove = !validMoves.empty();

	if ((blackScore + whiteScore == GRID_SIZE * GRID_SIZE) ||
		(!blackCanMove && !whiteCanMove)) {
		gameOver = true;
		gameOverTime = SDL_GetTicks();
	}
	else if (currentPlayer == 'B' && !blackCanMove) {
		passTurn = true;
		currentPlayer = 'W';
	}
	else if (currentPlayer == 'W' && !whiteCanMove) {
		passTurn = true;
		currentPlayer = 'B';
	}
	else {
		passTurn = false;
		FindValidMoves(currentPlayer);
	}
}

void CreatePieceTextures(SDL_Renderer* renderer) {
	// Load sprite sheet
	SDL_Surface* surface = IMG_Load("pieces.png");
	if (!surface) {
		std::cerr << "Failed to load sprite sheet: " << SDL_GetError() << std::endl;
		return;
	}

	// Create texture from surface
	pieceSpriteSheet = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);

	if (!pieceSpriteSheet) {
		std::cerr << "Failed to create texture from sprite sheet: " << SDL_GetError() << std::endl;
	}

	// Set texture blend mode for transparency if needed
	SDL_SetTextureBlendMode(pieceSpriteSheet, SDL_BLENDMODE_BLEND);
}

void RenderPiece(SDL_Renderer* renderer, int row, int col, char piece, float rotationAngle = 0.0f) {
	int centerX = GRID_OFFSET_X + col * CELL_SIZE + CELL_SIZE / 2;
	int centerY = GRID_OFFSET_Y + row * CELL_SIZE + CELL_SIZE / 2;
	int renderSize = CELL_SIZE - 10; // Slightly smaller than cell size

	SDL_Rect* srcRect = (piece == 'B') ? &blackPieceRect : &whitePieceRect;

	SDL_Rect dstRect = {
		centerX - renderSize / 2,
		centerY - renderSize / 2,
		renderSize,
		renderSize
	};

	// For flipping animation
	if (rotationAngle > 0.0f) {
		float scale = fabs(cos(rotationAngle));
		dstRect.w = static_cast<int>(renderSize * scale);
		dstRect.x = centerX - dstRect.w / 2;
	}

	SDL_Point center = { renderSize / 2, renderSize / 2 };
	SDL_RenderCopyEx(renderer, pieceSpriteSheet, srcRect, &dstRect,
		rotationAngle * 180.0f / PI, &center, SDL_FLIP_NONE);
}

void UpdateAnimations(Uint32 currentTime) {
	for (auto it = activeAnimations.begin(); it != activeAnimations.end(); ) {
		float progress = static_cast<float>(currentTime - it->startTime) / ANIMATION_DURATION_MS;

		if (progress >= 1.0f) {
			board[it->row][it->col] = it->endPiece;
			it = activeAnimations.erase(it);
			SoundSystem::PlaySound(SoundSystem::PIECE_FLIP);
		}
		else {
			++it;
		}
	}
}

void RenderValidMoves(SDL_Renderer* renderer) {
	if (gameOver) return;

	// Adicionado: Desativar hints durante o turno da IA
	if (currentGameMode == GameMode::VsAI && currentPlayer == 'W') {
		return;
	}

	SDL_SetRenderDrawColor(renderer, HINT_COLOR);

	for (const auto& move : validMoves) {
		int x = GRID_OFFSET_X + move.second * CELL_SIZE + CELL_SIZE / 2;
		int y = GRID_OFFSET_Y + move.first * CELL_SIZE + CELL_SIZE / 2;
		int radius = 5;

		for (int angle = 0; angle < 360; angle += 10) {
			double rad = angle * (M_PI / 180.0);
			int x1 = x + static_cast<int>(std::round(radius * cos(rad)));
			int y1 = y + static_cast<int>(std::round(radius * sin(rad)));
			int x2 = x + static_cast<int>(std::round(radius * cos(rad + 0.1)));
			int y2 = y + static_cast<int>(std::round(radius * sin(rad + 0.1)));
			SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
		}
	}
}

void RenderGameOver(SDL_Renderer* renderer, TTF_Font* font) {
	if (!gameOver) return;

	// Draw semi-transparent overlay covering the entire screen
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180); // Black with 70% opacity
	SDL_Rect overlay = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	SDL_RenderFillRect(renderer, &overlay);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

	char gameOverText[50];
	const char* winnerText;

	if (blackScore > whiteScore) {
		winnerText = GetGameStrings(currentLanguage).blackMessage;
	}
	else if (whiteScore > blackScore) {
		winnerText = GetGameStrings(currentLanguage).whiteMessage;
	}
	else {
		winnerText = "";
	}

	if (blackScore != whiteScore) {
		sprintf_s(gameOverText, GetGameStrings(currentLanguage).winMessage, winnerText);
	}
	else {
		sprintf_s(gameOverText, GetGameStrings(currentLanguage).drawMessage);
	}

	//RenderText(renderer, font, gameOverText, GetRelativeX(0.4), GetRelativeY(0.45));
	RenderTextWithSize(renderer, gameOverText, GetRelativeX(0.45f), GetRelativeY(0.4f), TEXT_SIZE);

	// Countdown to restart
	Uint32 currentTime = SDL_GetTicks();
	Uint32 elapsed = (currentTime - gameOverTime) / 1000;

	if (elapsed < RESTART_TIME) {
		char restartText[50];
		sprintf_s(restartText, GetGameStrings(currentLanguage).restartMessage, RESTART_TIME - elapsed);
		//RenderText(renderer, font, restartText, GetRelativeX(0.38), GetRelativeY(0.56));
		RenderTextWithSize(renderer, restartText, GetRelativeX(0.425f), GetRelativeY(0.6f), TEXT_SIZE);
	}
}

void HandleWindowResize(SDL_Window* window) {
	// Get new window dimensions
	SDL_GetWindowSize(window, &WINDOW_WIDTH, &WINDOW_HEIGHT);

	// Ensure minimum size
	WINDOW_WIDTH = max(WINDOW_WIDTH, MIN_WINDOW_WIDTH);
	WINDOW_HEIGHT = max(WINDOW_HEIGHT, MIN_WINDOW_HEIGHT);

	// Update window if needed
	int currentW, currentH;
	SDL_GetWindowSize(window, &currentW, &currentH);
	if (currentW != WINDOW_WIDTH || currentH != WINDOW_HEIGHT) {
		SDL_SetWindowSize(window, WINDOW_WIDTH, WINDOW_HEIGHT);
	}

	// Calculate available space for the grid (leave some margin)
	int availableWidth = WINDOW_WIDTH - 40;  // 20px margin on each side
	int availableHeight = WINDOW_HEIGHT - 120; // 120px margin for UI elements

	// Calculate cell size based on available space
	CELL_SIZE = std::min(availableWidth / GRID_SIZE, availableHeight / GRID_SIZE);

	// Ensure minimum cell size
	if (CELL_SIZE < 20) CELL_SIZE = 20;

	// Recalculate grid dimensions
	GRID_WIDTH = CELL_SIZE * GRID_SIZE;
	GRID_HEIGHT = CELL_SIZE * GRID_SIZE;

	// Center the grid
	GRID_OFFSET_X = (WINDOW_WIDTH - GRID_WIDTH) / 2;
	GRID_OFFSET_Y = (WINDOW_HEIGHT - GRID_HEIGHT) / 2; // Offset for UI
}

void EventHandler(GameState& currentState, SDL_Window* window) {
	while (SDL_PollEvent(&event)) {
		HandleCheatCodes(event, currentState, board, currentPlayer, gameOver);

		if (event.type == SDL_QUIT) {
			quit = true;
		}
		else if (event.type == SDL_WINDOWEVENT) {
			if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
				HandleWindowResize(window);
				SDL_RenderPresent(SDL_GetRenderer(window));
			}
		}
		else if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {
			case SDLK_t:
				ResetGame();
				currentState = GameState::TITLE_SCREEN;
				break;

			case SDLK_ESCAPE:
				quit = true;
				break;
			}
		}
		else if (event.type == SDL_MOUSEBUTTONDOWN && !gameOver && !passTurn) {
			if (!activeAnimations.empty()) {
				continue;
			}

			int mouseX, mouseY;
			SDL_GetMouseState(&mouseX, &mouseY);

			if (mouseX >= GRID_OFFSET_X && mouseX < GRID_OFFSET_X + GRID_WIDTH &&
				mouseY >= GRID_OFFSET_Y && mouseY < GRID_OFFSET_Y + GRID_HEIGHT) {

				int row = (mouseY - GRID_OFFSET_Y) / CELL_SIZE;
				int col = (mouseX - GRID_OFFSET_X) / CELL_SIZE;

				if (IsValidMove(row, col, currentPlayer)) {
					// Atualiza o tabuleiro primeiro
					MakeMove(row, col, currentPlayer);
					// Muda o jogador
					currentPlayer = (currentPlayer == 'B') ? 'W' : 'B';
					UpdateGameState();
				}
			}
		}
	}
}

int main(int argc, char* argv[]) {
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		cout << "SDL Initialization Error: " << SDL_GetError() << endl;
		return -1;
	}

	if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
		cout << "SDL_image could not initialize! Error: " << IMG_GetError() << endl;
		SDL_Quit();
		return -1;
	}

	if (TTF_Init() == -1) {
		cout << "TTF Initialization Error: " << TTF_GetError() << endl;
		SDL_Quit();
		return -1;
	}

	// Create window (dinamically)
	SDL_Window* window = SDL_CreateWindow(
		GetGameStrings(currentLanguage).windowName,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED		//SDL_WINDOW_FULLSCREEN_DESKTOP
	);

	SetWindowMinMaxSize(window);

	// Initialize sound system
	if (!SoundSystem::Initialize()) {
		cout << "Warning: Sound system failed to initialize. Continuing without sound." << endl;
	}
	SoundSystem::LoadSounds();

	// Create renderer
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	CreatePieceTextures(renderer);

	if (!window || !renderer) {
		cout << "Window or Renderer Error: " << SDL_GetError() << endl;
		SDL_Quit();
		return -1;
	}

	// Load font
	TTF_Font* font = TTF_OpenFont(TEXT_FONT, TEXT_SIZE);
	if (!font) {
		cout << "Font Loading Error: " << TTF_GetError() << endl;
		cout << "Trying to load: " << TEXT_FONT << endl;
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		TTF_Quit();
		SDL_Quit();
		return -1;
	}

	// Game state variables
	GameState currentState = GameState::TITLE_SCREEN;
	ResetGame();

	// Main game loop
	while (!quit) {
		Uint32 currentTime = SDL_GetTicks();

		if (currentState == GameState::TITLE_SCREEN) {
			HandleTitleScreenEvents(event, currentState, quit, currentLanguage, window);
			RenderTitleScreen(renderer, font, currentLanguage, currentTime, pieceSpriteSheet);
		}
		else if (currentState == GameState::GAME_SCREEN) {
			EventHandler(currentState, window);
			UpdateAnimations(currentTime);

			if (!gameOver) {
				UpdateGameState();

				// Lógica da AI (modo 1 jogador)
				if (currentGameMode == GameMode::VsAI && currentPlayer == 'W' && !passTurn) {
					if (activeAnimations.empty()) {
						AI ai(AIDifficulty::EASY);
						auto move = ai.MakeMove(board, currentPlayer);
						if (move.first != -1 && move.second != -1) {
							MakeMove(move.first, move.second, currentPlayer); // <--- CHANGE IS HERE
							currentPlayer = 'B'; // AI (White) just moved, so switch to Black
							UpdateGameState();
						}
					}
				}
			}
			else {
				// Lógica de game over
				Uint32 currentTime = SDL_GetTicks();
				if (currentTime - gameOverTime >= RESTART_TIME * 1000) {
					ResetGame();
				}
				if (gameOver && gameOverTime + 100 > currentTime) {
					SoundSystem::PlaySound(SoundSystem::GAME_OVER);
				}
			}

			// Clear screen (green background)
			SDL_SetRenderDrawColor(renderer, GAME_BACKGROUND_COLOR);
			SDL_RenderClear(renderer);

			// Render score
			char score_text[100];
			sprintf_s(score_text, GetGameStrings(currentLanguage).scoreText, blackScore, whiteScore);
			//RenderText(renderer, font, score_text, GetRelativeX(0.02), GetRelativeY(0.02));
			RenderTextWithSize(renderer, score_text, GetRelativeX(0.02f), GetRelativeY(0.02f), TEXT_SIZE);

			// Draw Grid (dark green lines)
			SDL_SetRenderDrawColor(renderer, GRID_COLOR);
			for (int i = 0; i <= GRID_SIZE; i++) {
				// Vertical lines
				SDL_RenderDrawLine(
					renderer,
					GRID_OFFSET_X + i * CELL_SIZE,
					GRID_OFFSET_Y,
					GRID_OFFSET_X + i * CELL_SIZE,
					GRID_OFFSET_Y + GRID_HEIGHT
				);

				// Horizontal lines
				SDL_RenderDrawLine(
					renderer,
					GRID_OFFSET_X,
					GRID_OFFSET_Y + i * CELL_SIZE,
					GRID_OFFSET_X + GRID_WIDTH,
					GRID_OFFSET_Y + i * CELL_SIZE
				);
			}

			// Draw pieces
			for (int row = 0; row < GRID_SIZE; row++) {
				for (int col = 0; col < GRID_SIZE; col++) {
					bool isAnimating = false;
					float rotationAngle = 0.0f;

					for (const auto& anim : activeAnimations) {
						if (anim.row == row && anim.col == col) {
							isAnimating = true;
							float progress = static_cast<float>(currentTime - anim.startTime) / ANIMATION_DURATION_MS;
							// Garantir que o progresso fique entre 0 e 1
							progress = std::min(std::max(progress, 0.0f), 1.0f);
							float rotationAngle = progress * PI; // Full flip is 180 degrees

							// Determinar qual lado mostrar baseado no progresso
							char displayPiece = (progress < 0.5f) ? anim.startPiece : anim.endPiece;
							RenderPiece(renderer, row, col, displayPiece, rotationAngle);
							break;
						}
					}

					if (!isAnimating && (board[row][col] == 'B' || board[row][col] == 'W')) {
						RenderPiece(renderer, row, col, board[row][col]);
					}
				}
			}

			// Show valid moves
			if (!gameOver && !passTurn) {
				RenderValidMoves(renderer);
			}

			// Render current player turn or game over message
			if (!gameOver) {
				char playerTurn_text[100];
				const char* playerName = (currentPlayer == 'B') ?
					GetGameStrings(currentLanguage).blackMessage :
					GetGameStrings(currentLanguage).whiteMessage;

				sprintf_s(playerTurn_text, GetGameStrings(currentLanguage).turnText, playerName);
				//RenderText(renderer, font, playerTurn_text, GetRelativeX(0.45), GetRelativeY(0.02));
				RenderTextWithSize(renderer, playerTurn_text, GetRelativeX(0.45f), GetRelativeY(0.02f), TEXT_SIZE);

				if (passTurn) {
					//RenderText(renderer, font, GetGameStrings(currentLanguage).noMovesMessage, GetRelativeX(0.02), GetRelativeY(0.92));
					RenderTextWithSize(renderer, GetGameStrings(currentLanguage).noMovesMessage, GetRelativeX(0.02f), GetRelativeY(0.92f), TEXT_SIZE);
				}

				//RenderText(renderer, font, GetGameStrings(currentLanguage).returnTitleMessage, GetRelativeX(0.02), GetRelativeY(0.92));
				RenderTextWithSize(renderer, GetGameStrings(currentLanguage).returnTitleMessage, GetRelativeX(0.02f), GetRelativeY(0.92f), TEXT_SIZE);
			}
			else {
				RenderGameOver(renderer, font);
			}

			SDL_RenderPresent(renderer);

			// Add frame rate limiting (e.g., to 60 FPS)
			const int FPS = 60;
			const int frameDelay = 1000 / FPS;
			Uint32 frameTime = SDL_GetTicks() - currentTime;
			if (frameDelay > frameTime) {
				SDL_Delay(frameDelay - frameTime);
			}
		}
	}

	// Cleanup
	SoundSystem::Shutdown();
	TTF_CloseFont(font);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	IMG_Quit();

	if (pieceSpriteSheet) {
		SDL_DestroyTexture(pieceSpriteSheet);
	}

	TTF_Quit();
	SDL_Quit();
	return 0;
}