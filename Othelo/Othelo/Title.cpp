#include "Title.h"
#include "Main.h"
#include "Sound.h"
#include <SDL.h>
#include <SDL_ttf.h>
// Overloaded version that accepts a custom font size
void RenderTextWithSize(SDL_Renderer* renderer, const char* text, int x, int y, int fontSize) {
    TTF_Font* customFont = TTF_OpenFont(TEXT_FONT, fontSize);
    if (!customFont) {
        std::cerr << "Failed to load font with size " << fontSize << ": " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Color color = { TEXT_COLOR };
    SDL_Surface* surface = TTF_RenderUTF8_Blended(customFont, text, color);
    if (!surface) {
        std::cerr << "Failed to render text: " << TTF_GetError() << std::endl;
        TTF_CloseFont(customFont);
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(surface);
        TTF_CloseFont(customFont);
        return;
    }

    SDL_Rect rect = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    TTF_CloseFont(customFont);
}

void RenderTitlePieces(SDL_Renderer* renderer, SDL_Texture* pieceTexture, Uint32 currentTime) {
    const int centerX = WINDOW_WIDTH / 2;
    const int centerY = WINDOW_HEIGHT / 2;

    const int pieceSize = static_cast<int>(WINDOW_HEIGHT * 0.15f);
    const int pieceSpacing = static_cast<int>(WINDOW_HEIGHT * 0.04f);

    SDL_Rect blackPieceRect = { 0, 0, 300, 300 };
    SDL_Rect whitePieceRect = { 300, 0, 300, 300 };

    // Calculate total width and starting position
    int totalWidth = 3 * pieceSize + 2 * pieceSpacing;
    int startX = centerX - totalWidth / 2;

    // Calculate current animation step (0-4)
    int animationStep = (currentTime % TITLE_FULL_CYCLE_DURATION) / TITLE_ANIMATION_STEP_DURATION;

    // Left black piece (appears in step 1)
    if (animationStep >= 1) {
        SDL_Rect leftDst = {
            startX,
            centerY - pieceSize / 2,
            pieceSize,
            pieceSize
        };
        SDL_RenderCopy(renderer, pieceTexture, &blackPieceRect, &leftDst);
    }

    // Middle white piece (appears in step 2, animates in step 4)
    if (animationStep >= 2) {
        SDL_Rect middleDst = {
            startX + pieceSize + pieceSpacing,
            centerY - pieceSize / 2,
            pieceSize,
            pieceSize
        };
        SDL_RenderCopy(renderer, pieceTexture, &whitePieceRect, &middleDst);

        if (animationStep == 4) {
            Uint32 stepTime = currentTime % TITLE_ANIMATION_STEP_DURATION;
            float progress = static_cast<float>(stepTime) / TITLE_ANIMATION_STEP_DURATION;
            float rotationAngle = progress * M_PI;

            SDL_Point center = { pieceSize / 2, pieceSize / 2 };
            SDL_RenderCopyEx(renderer, pieceTexture, &blackPieceRect, &middleDst,
                (rotationAngle - M_PI / 2) * 180.0f / M_PI, &center, SDL_FLIP_NONE);
        }
    }

    // Right black piece (appears in step 3)
    if (animationStep >= 3) {
        SDL_Rect rightDst = {
            startX + 2 * (pieceSize + pieceSpacing),
            centerY - pieceSize / 2,
            pieceSize,
            pieceSize
        };
        SDL_RenderCopy(renderer, pieceTexture, &blackPieceRect, &rightDst);
    }
}

void RenderTitleScreen(SDL_Renderer* renderer, TTF_Font* font, Language currentLanguage, Uint32 currentTime, SDL_Texture* pieceTexture) {
    // Clear screen with a dark background
    SDL_SetRenderDrawColor(renderer, 0, 50, 0, 255);
    SDL_RenderClear(renderer);

    // Render pieces animation
    RenderTitlePieces(renderer, pieceTexture, currentTime);

    const GameStrings& gameStrings = GetGameStrings(currentLanguage);
    const TitleStrings& titleStrings = GetTitleStrings(currentLanguage);

    // Calculate dynamic positions based on window size
    const int centerX = WINDOW_WIDTH / 2;
    const int centerY = WINDOW_HEIGHT / 2;

    // Render game title with dynamic font size
    int titleFontSize = GetTitleFontSize()*2;
    TTF_Font* titleFont = TTF_OpenFont(TEXT_FONT, titleFontSize);
    if (titleFont) {
        SDL_Surface* titleSurface = TTF_RenderUTF8_Blended(titleFont, gameStrings.windowName, { TEXT_COLOR });
        if (titleSurface) {
            SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
            if (titleTexture) {
                SDL_Rect titleRect = {
                    centerX - titleSurface->w / 2,
                    static_cast<int>(centerY - WINDOW_HEIGHT * 0.35f),
                    titleSurface->w,
                    titleSurface->h
                };
                SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);
                SDL_DestroyTexture(titleTexture);
            }
            SDL_FreeSurface(titleSurface);
        }
        TTF_CloseFont(titleFont);
    }

    // Create font for regular text with dynamic size
    int regularFontSize = GetRegularFontSize();
    TTF_Font* regularFont = TTF_OpenFont(TEXT_FONT, regularFontSize);
    if (!regularFont) {
        regularFont = font; // Fallback to the provided font
    }

    // Blinking "Press to Start" message
    bool showText = (currentTime / BLINK_INTERVAL_MS) % 2 == 0;
    if (showText) {
        SDL_Surface* startSurface = TTF_RenderUTF8_Blended(regularFont, titleStrings.pressToStart, { TEXT_COLOR });
        if (startSurface) {
            SDL_Texture* startTexture = SDL_CreateTextureFromSurface(renderer, startSurface);
            if (startTexture) {
                SDL_Rect startRect = {
                    centerX - startSurface->w / 2,
                    static_cast<int>(centerY + WINDOW_HEIGHT * 0.10f),
                    startSurface->w,
                    startSurface->h
                };
                SDL_RenderCopy(renderer, startTexture, NULL, &startRect);
                SDL_DestroyTexture(startTexture);
            }
            SDL_FreeSurface(startSurface);
        }
    }

    // Render game mode selection
    const char* modeText = (currentGameMode == GameMode::TwoPlayers) ?
        titleStrings.twoPlayersMode : titleStrings.vsAIMode;
    SDL_Surface* modeSurface = TTF_RenderUTF8_Blended(regularFont, modeText, { TEXT_COLOR });
    if (modeSurface) {
        SDL_Texture* modeTexture = SDL_CreateTextureFromSurface(renderer, modeSurface);
        if (modeTexture) {
            SDL_Rect modeRect = {
                centerX - modeSurface->w / 2,
                static_cast<int>(centerY + WINDOW_HEIGHT * 0.18f),
                modeSurface->w,
                modeSurface->h
            };
            SDL_RenderCopy(renderer, modeTexture, NULL, &modeRect);
            SDL_DestroyTexture(modeTexture);
        }
        SDL_FreeSurface(modeSurface);
    }

    // Render language selection
    SDL_Surface* langSurface = TTF_RenderUTF8_Blended(regularFont, titleStrings.languageOption, { TEXT_COLOR });
    if (langSurface) {
        SDL_Texture* langTexture = SDL_CreateTextureFromSurface(renderer, langSurface);
        if (langTexture) {
            SDL_Rect langRect = {
                centerX - langSurface->w / 2,
                static_cast<int>(centerY + WINDOW_HEIGHT * 0.26f),
                langSurface->w,
                langSurface->h
            };
            SDL_RenderCopy(renderer, langTexture, NULL, &langRect);
            SDL_DestroyTexture(langTexture);
        }
        SDL_FreeSurface(langSurface);
    }

    // Render quit instruction
    SDL_Surface* quitSurface = TTF_RenderUTF8_Blended(regularFont, titleStrings.pressToQuit, { TEXT_COLOR });
    if (quitSurface) {
        SDL_Texture* quitTexture = SDL_CreateTextureFromSurface(renderer, quitSurface);
        if (quitTexture) {
            SDL_Rect quitRect = {
                centerX - quitSurface->w / 2,
                static_cast<int>(centerY + WINDOW_HEIGHT * 0.34f),
                quitSurface->w,
                quitSurface->h
            };
            SDL_RenderCopy(renderer, quitTexture, NULL, &quitRect);
            SDL_DestroyTexture(quitTexture);
        }
        SDL_FreeSurface(quitSurface);
    }

    // Clean up dynamic font if it was created
    if (regularFont != font) {
        TTF_CloseFont(regularFont);
    }

    SDL_RenderPresent(renderer);
}

void HandleTitleScreenEvents(SDL_Event& event, GameState& currentState, bool& quit, Language& currentLanguage, SDL_Window* window) {
    while (SDL_PollEvent(&event)) {
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
            case SDLK_SPACE:
                SoundSystem::PlaySound(SoundSystem::MENU_SELECT);
                currentState = GameState::GAME_SCREEN;
                break;
            case SDLK_l:
                SoundSystem::PlaySound(SoundSystem::MENU_CHANGE);
                // Cycle to next language
                currentLanguage = static_cast<Language>(
                    (static_cast<int>(currentLanguage) + 1) % static_cast<int>(Language::LANGUAGE_COUNT)
                    );
                break;
            case SDLK_p:
                SoundSystem::PlaySound(SoundSystem::MENU_CHANGE);
                currentGameMode = (currentGameMode == GameMode::TwoPlayers) ?
                    GameMode::VsAI : GameMode::TwoPlayers;
                break;
            case SDLK_ESCAPE:
            case SDLK_q:
                quit = true;
                break;
            }
        }
    }
}