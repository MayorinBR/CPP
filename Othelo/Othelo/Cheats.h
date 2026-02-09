#pragma once
#include <vector>
#include <SDL.h>
#include "Title.h"

#define CHEAT_CODE_WIN          "win"
#define CHEAT_CODE_CLEAR        "clear"
#define CHEAT_CODE_SWITCH       "gameshark"
#define CHEAT_CODE_CHANGE       "change"
#define CHEAT_BUFFER_LENGHT     10

void HandleCheatCodes(SDL_Event& event, GameState currentState,
	std::vector<std::vector<char>>& board,
	char& currentPlayer, bool& gameOver);