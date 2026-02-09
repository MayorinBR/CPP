#include "Title.h"
#include "Cheats.h"
#include <SDL.h>
#include <iostream>
#include <vector>
#include <string>

// Internal cheat buffer for string-based cheat codes
static std::string cheatBuffer;
static bool ctrlHeld = false;

void HandleCheatCodes(SDL_Event& event, GameState currentState,
	std::vector<std::vector<char>>& board,
	char& currentPlayer, bool& gameOver)
{
	if (currentState != GameState::GAME_SCREEN || gameOver) return;

	if (event.type == SDL_KEYDOWN) {
		SDL_Keycode key = event.key.keysym.sym;

		// Check if Ctrl is being held
		ctrlHeld = (event.key.keysym.mod & KMOD_CTRL);

		if (ctrlHeld && key >= SDLK_a && key <= SDLK_z) {
			char c = static_cast<char>(key);
			cheatBuffer += c;
			std::cout << "Cheat buffer: " << cheatBuffer << "\n";

			// Match known cheat codes
			if (cheatBuffer == CHEAT_CODE_WIN) {
				std::cout << "Cheat activated: Instant Win\n";
				for (int i = 0; i < GRID_SIZE; ++i) {
					for (int j = 0; j < GRID_SIZE; ++j) {
						board[i][j] = currentPlayer;
					}
				}	
				cheatBuffer.clear();
			}
			else if (cheatBuffer == CHEAT_CODE_CLEAR) {
				std::cout << "Cheat activated: Board Clear\n";
				for (auto& row : board)
					for (char& cell : row)
						cell = ' ';
				currentPlayer = 'X';
				cheatBuffer.clear();
			}
			else if (cheatBuffer == CHEAT_CODE_SWITCH) {
				std::cout << "Cheat activated: Player Switch\n";
				currentPlayer = (currentPlayer == 'B') ? 'W' : 'B';
				std::cout << "Current player is now: " << currentPlayer << "\n";
				cheatBuffer.clear();
			}
			else if (cheatBuffer == CHEAT_CODE_CHANGE) {
				std::cout << "Cheat activated: Space Swapper\n";
				int mouseX, mouseY;
				SDL_GetMouseState(&mouseX, &mouseY);

				if (mouseX >= GRID_OFFSET_X && mouseX < GRID_OFFSET_X + GRID_WIDTH &&
					mouseY >= GRID_OFFSET_Y && mouseY < GRID_OFFSET_Y + GRID_HEIGHT) {

					int row = (mouseY - GRID_OFFSET_Y) / CELL_SIZE;
					int col = (mouseX - GRID_OFFSET_X) / CELL_SIZE;

					if (board[row][col] != ' ') {
						board[row][col] = currentPlayer;
						std::cout << "Swapped mark at (" << row << "," << col << ")\n";
					}
				}
			}
			else if (cheatBuffer.length() > CHEAT_BUFFER_LENGHT) {
				// Reset buffer if too long to prevent abuse
				cheatBuffer.clear();
			}
		}
		else if (!ctrlHeld) {
			// Clear buffer if Ctrl is not held
			cheatBuffer.clear();
		}
	}
	else if (event.type == SDL_KEYUP) {
		if (event.key.keysym.sym == SDLK_LCTRL || event.key.keysym.sym == SDLK_RCTRL) {
			ctrlHeld = false;
			cheatBuffer.clear();
		}
	}
}