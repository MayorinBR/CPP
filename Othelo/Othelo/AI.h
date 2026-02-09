#pragma once
#include <vector>
#include <utility>
#include <algorithm>
#include <random>
#include <chrono>

// Keep only EASY difficulty
enum class AIDifficulty {
	EASY
};

class AI {
public:
	// Constructor now only needs difficulty
	AI(AIDifficulty difficulty = AIDifficulty::EASY);
	void SetDifficulty(AIDifficulty difficulty); // Can still be used to set EASY
	std::pair<int, int> MakeMove(const std::vector<std::vector<char>>& board, char player);

private:
	AIDifficulty currentDifficulty;

	// Only keep MakeRandomMove
	std::pair<int, int> MakeRandomMove(const std::vector<std::vector<char>>& board, char player);

	// Keep helper functions used by MakeRandomMove
	std::vector<std::pair<int, int>> GetValidMoves(const std::vector<std::vector<char>>& board, char player);
	bool IsValidMove(const std::vector<std::vector<char>>& board, int row, int col, char player);
};