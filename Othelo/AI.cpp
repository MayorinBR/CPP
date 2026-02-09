#include "AI.h"
#include <SDL.h>

AI::AI(AIDifficulty difficulty) : currentDifficulty(difficulty) {}

void AI::SetDifficulty(AIDifficulty difficulty) {
	currentDifficulty = difficulty;
}

std::pair<int, int> AI::MakeMove(const std::vector<std::vector<char>>& board, char player) {
	// Now only calls MakeRandomMove since other difficulties are removed
	SDL_Delay(1000);
	return MakeRandomMove(board, player);
}

std::pair<int, int> AI::MakeRandomMove(const std::vector<std::vector<char>>& board, char player) {
	auto validMoves = GetValidMoves(board, player);
	if (validMoves.empty()) return { -1, -1 }; // No valid moves

	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::shuffle(validMoves.begin(), validMoves.end(), std::default_random_engine(seed));
	return validMoves[0];
}

// Helper functions (kept as they are needed by MakeRandomMove)
std::vector<std::pair<int, int>> AI::GetValidMoves(const std::vector<std::vector<char>>& board, char player) {
	std::vector<std::pair<int, int>> validMoves;
	for (int row = 0; row < 8; ++row) {
		for (int col = 0; col < 8; ++col) {
			if (IsValidMove(board, row, col, player)) {
				validMoves.emplace_back(row, col);
			}
		}
	}
	return validMoves;
}

bool AI::IsValidMove(const std::vector<std::vector<char>>& board, int row, int col, char player) {
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

			while (r >= 0 && r < 8 && c >= 0 && c < 8) {
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