// Title.cpp
#include "Title.h"
#include "Main.h"
#include "Sound.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <iostream>
#include "Highscore.h"
#include "Settings.h"
#include <SDL_keyboard.h> // Necessário para gerenciar a entrada de texto

SDL_Texture* titleTexture = nullptr;

extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;
extern Difficulty currentDifficulty;
extern Language currentLanguage;
static int currentLanguageIndex = 0; // 0: English, 1: Japanese, 2: Portuguese
static int currentMenuSelection = 0;
SDL_Color textColor = { 255, 255, 255, 255 };

// Function to load the title texture
bool LoadTitleTexture(SDL_Renderer* renderer) {
    titleTexture = IMG_LoadTexture(renderer, TITLE_IMAGE_FILE); // Carrega a textura
    if (titleTexture == nullptr) {
        std::cerr << "Failed to load title texture: " << IMG_GetError() << std::endl;
        return false;
    }
    return true;
}

// Function to cleanup the title texture
void CleanupTitleTexture() {
    if (titleTexture) {
        SDL_DestroyTexture(titleTexture);
        titleTexture = nullptr;
    }
}

// Auxiliary function to render centered text
void RenderCenteredText(SDL_Renderer* renderer, const char* text, int y, int fontSize) {
    TTF_Font* font = TTF_OpenFont(TEXT_FONT, fontSize);
    if (font == nullptr) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Color color = { TEXT_COLOR };
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, color);
    if (surface == nullptr) {
        std::cerr << "Failed to create text surface: " << TTF_GetError() << std::endl;
        TTF_CloseFont(font);
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == nullptr) {
        std::cerr << "Failed to create texture from surface: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(surface);
        TTF_CloseFont(font);
        return;
    }

    SDL_Rect textRect;
    textRect.w = surface->w;
    textRect.h = surface->h;
    textRect.x = (WINDOW_WIDTH - textRect.w) / 2;
    textRect.y = y;
    SDL_RenderCopy(renderer, texture, NULL, &textRect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
    TTF_CloseFont(font);
}

// Auxiliary function to render text at a specific position
void RenderTextAtPosition(SDL_Renderer* renderer, const std::string& text, int x, int y, int fontSize, SDL_Color color) {
    TTF_Font* font = TTF_OpenFont(TEXT_FONT, fontSize);
    if (font == nullptr) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (surface == nullptr) {
        std::cerr << "Failed to create text surface: " << TTF_GetError() << std::endl;
        TTF_CloseFont(font);
        return;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == nullptr) {
        std::cerr << "Failed to create texture from surface: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(surface);
        TTF_CloseFont(font);
        return;
    }

    SDL_Rect textRect = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &textRect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
    TTF_CloseFont(font);
}

// Renders the title screen
void RenderTitleScreen(SDL_Renderer* renderer) {
    int menuY = WINDOW_HEIGHT / 2; // Posição inicial do menu (fallback/default)
    int optionSpacing = 60; //

    if (titleTexture) {
        // Lógica de escalonamento para 1536x1024 (aspect ratio 1.5)
        float imageAspectRatio = (float)TITLE_IMAGE_WIDTH / (float)TITLE_IMAGE_HEIGHT; // 1.5

        // Determina a altura alvo (ex: 40% da altura da janela)
        int targetHeight = (int)(WINDOW_HEIGHT * 0.40);

        // Calcula a largura alvo mantendo o aspect ratio
        int targetWidth = (int)(targetHeight * imageAspectRatio);

        // Garante que a imagem caiba na largura da tela (com margem)
        int maxAllowedWidth = WINDOW_WIDTH - (int)(WINDOW_WIDTH * 0.05); // Margem de 5%
        if (targetWidth > maxAllowedWidth) {
            targetWidth = maxAllowedWidth;
            targetHeight = (int)(targetWidth / imageAspectRatio);
        }

        // Calcula a posição para centralizar horizontalmente e posicionar verticalmente no topo
        int imageY = (int)(WINDOW_HEIGHT * 0.10); // 10% do topo
        int imageX = (WINDOW_WIDTH - targetWidth) / 2;

        SDL_Rect destRect = { imageX, imageY, targetWidth, targetHeight };
        SDL_RenderCopy(renderer, titleTexture, NULL, &destRect);

        // Define a nova posição Y para o início do menu, abaixo da imagem (com margem de 30px)
        menuY = imageY + targetHeight + 30;
    }
    else {
        // Fallback: Se a imagem não carregar, renderiza o texto original do título
        RenderCenteredText(renderer, LanguageManager::getText("gameTitle").c_str(), WINDOW_HEIGHT / 4, 70); //
        // menuY permanece em WINDOW_HEIGHT / 2
    }

    // Opção Iniciar Jogo
    std::string startText = LanguageManager::getText("startGame"); //
    if (currentMenuSelection == 0) { //
        startText = "> " + startText + " <"; //
    }
    RenderCenteredText(renderer, startText.c_str(), menuY, 40); // Usa o novo/ajustado menuY

    // Opção Recordes
    std::string highscoreText = LanguageManager::getText("highscores"); //
    if (currentMenuSelection == 1) { //
        highscoreText = "> " + highscoreText + " <"; //
    }
    RenderCenteredText(renderer, highscoreText.c_str(), menuY + optionSpacing, 40); //

    // Opção Configurações
    std::string settingsText = LanguageManager::getText("settings"); //
    if (currentMenuSelection == 2) { //
        settingsText = "> " + settingsText + " <"; //
    }
    RenderCenteredText(renderer, settingsText.c_str(), menuY + (optionSpacing * 2), 40); //

    // Opção Sair do Jogo
    std::string quitText = LanguageManager::getText("quitGame"); //
    if (currentMenuSelection == 3) { //
        quitText = "> " + quitText + " <"; //
    }
    RenderCenteredText(renderer, quitText.c_str(), menuY + (optionSpacing * 3), 40); //
}


// Renders the difficulty selection screen
void RenderDifficultyScreen(SDL_Renderer* renderer) {
    RenderCenteredText(renderer, LanguageManager::getText("difficultySelect").c_str(), WINDOW_HEIGHT / 4, 40);

    // Opção Fácil
    std::string easyText = LanguageManager::getText("easy");
    if (currentDifficulty == Difficulty::EASY) {
        easyText = "> " + easyText + " <";
    }
    RenderCenteredText(renderer, easyText.c_str(), WINDOW_HEIGHT / 2, 40);

    // Opção Normal
    std::string normalText = LanguageManager::getText("normal");
    if (currentDifficulty == Difficulty::NORMAL) {
        normalText = "> " + normalText + " <";
    }
    RenderCenteredText(renderer, normalText.c_str(), WINDOW_HEIGHT / 2 + 60, 40);

    // Opção Difícil
    std::string hardText = LanguageManager::getText("hard");
    if (currentDifficulty == Difficulty::HARD) {
        hardText = "> " + hardText + " <";
    }
    RenderCenteredText(renderer, hardText.c_str(), WINDOW_HEIGHT / 2 + 120, 40);
}

// Renders the language selection screen
void RenderLanguageScreen(SDL_Renderer* renderer) {
    RenderCenteredText(renderer, LanguageManager::getText("languageSelect").c_str(), WINDOW_HEIGHT / 8, 50);

    // Get number of languages and calculate position
    int numLanguages = static_cast<int>(Language::LANGUAGE_COUNT);
    int flagSize = (int)(MIN_WINDOW_WIDTH / FLAG_SIZE_FACTOR);
    int totalWidth = numLanguages * flagSize;
    int spacing = 50;
    int startX = (WINDOW_WIDTH - (totalWidth + (numLanguages - 1) * spacing)) / 2;
    int startY = (WINDOW_HEIGHT / 2) - (flagSize / 2);

    for (int i = 0; i < static_cast<int>(Language::LANGUAGE_COUNT); ++i) {
        SDL_Surface* flagSurface = IMG_Load(FLAG_FILES[i]);
        if (flagSurface == nullptr) {
            std::cerr << "Failed to load flag image: " << IMG_GetError() << std::endl;
            continue;
        }

        SDL_Texture* flagTexture = SDL_CreateTextureFromSurface(renderer, flagSurface);
        if (flagTexture == nullptr) {
            std::cerr << "Failed to create texture from surface: " << SDL_GetError() << std::endl;
            SDL_FreeSurface(flagSurface);
            continue;
        }

        SDL_Rect flagRect = { startX + i * (flagSize + spacing), startY, flagSize, flagSize };
        SDL_RenderCopy(renderer, flagTexture, NULL, &flagRect);

        // Highlight the selected language
        if (i == static_cast<int>(currentLanguage)) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);
            SDL_Rect highlightRect = { flagRect.x - 5, flagRect.y - 5, flagRect.w + 10, flagRect.h + 10 };
            SDL_RenderDrawRect(renderer, &highlightRect);
        }

        SDL_DestroyTexture(flagTexture);
        SDL_FreeSurface(flagSurface);
    }

    // Render the language name below the flag
    int optionFontSize = 32;
    std::string langText = "";
    switch (currentLanguage) {
    case Language::ENGLISH: langText = LanguageManager::getText("english"); break;
    case Language::JAPANESE: langText = LanguageManager::getText("japanese"); break;
    case Language::PORTUGUESE: langText = LanguageManager::getText("portuguese"); break;
    default: break;
    }
    RenderCenteredText(renderer, langText.c_str(), startY + flagSize + 10, optionFontSize);

    // Render the "Press Enter" message
    RenderCenteredText(renderer, LanguageManager::getText("pressEnter").c_str(), WINDOW_HEIGHT - 50, optionFontSize);
}

// Handle language selection screen events
void HandleLanguageScreenEvents(SDL_Event& event, GameState& currentState, bool& quit) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        case SDLK_LEFT:
            currentLanguage = static_cast<Language>((static_cast<int>(currentLanguage) - 1 + static_cast<int>(Language::LANGUAGE_COUNT)) % static_cast<int>(Language::LANGUAGE_COUNT));
            SoundSystem::PlaySound(SoundSystem::SoundType::MENU_SELECT);
            break;
        case SDLK_RIGHT:
            currentLanguage = static_cast<Language>((static_cast<int>(currentLanguage) + 1) % static_cast<int>(Language::LANGUAGE_COUNT));
            SoundSystem::PlaySound(SoundSystem::SoundType::MENU_SELECT);
            break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            SoundSystem::PlaySound(SoundSystem::SoundType::MENU_SELECT);
            currentState = GameState::TITLE_SCREEN;
            SoundSystem::PlayBgm(0);
            break;
        case SDLK_ESCAPE:
        case SDLK_q:
            quit = true;
            break;
        }
    }
}

// Handle title screen events
void HandleTitleScreenEvents(SDL_Event& event, GameState& currentState, bool& quit) {
    if (event.type == SDL_KEYDOWN) {
        SoundSystem::PlaySound(SoundSystem::SoundType::MENU_SELECT);
        switch (event.key.keysym.sym) {
        case SDLK_UP:
            currentMenuSelection = (currentMenuSelection - 1 + 4) % 4;
            break;
        case SDLK_DOWN:
            currentMenuSelection = (currentMenuSelection + 1) % 4;
            break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            if (currentMenuSelection == 0) { // Start Game
                currentState = GameState::DIFFICULTY_SCREEN;
            }
            else if (currentMenuSelection == 1) { // Highscores
                currentState = GameState::HIGHSCORES_SCREEN;
            }
            else if (currentMenuSelection == 2) { // Settings
                currentState = GameState::SETTINGS_SCREEN;
            }
            else if (currentMenuSelection == 3) { // Quit
                quit = true;
            }
            break;
        case SDLK_ESCAPE:
        case SDLK_q:
            quit = true;
            break;
        }
    }
}

// Handle difficulty selection screen events
void HandleDifficultyScreenEvents(SDL_Event& event, GameState& currentState) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        case SDLK_UP:
            currentDifficulty = static_cast<Difficulty>((static_cast<int>(currentDifficulty) - 1 + 3) % 3);
            SoundSystem::PlaySound(SoundSystem::SoundType::MENU_SELECT);
            break;
        case SDLK_DOWN:
            currentDifficulty = static_cast<Difficulty>((static_cast<int>(currentDifficulty) + 1) % 3);
            SoundSystem::PlaySound(SoundSystem::SoundType::MENU_SELECT);
            break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            NewGame(currentDifficulty);
            SoundSystem::PlaySound(SoundSystem::SoundType::MENU_SELECT);
            currentState = GameState::GAME_SCREEN;
            break;
        case SDLK_ESCAPE:
            currentState = GameState::TITLE_SCREEN;
            break;
        }
    }
}

// Renders the highscores screen
void RenderHighscoresScreen(SDL_Renderer* renderer) {
    RenderCenteredText(renderer, LanguageManager::getText("highscoresTitle").c_str(), (WINDOW_HEIGHT / 8) - 20, 50);

    SDL_Color gold = { 255, 215, 0, 255 };
    SDL_Color silver = { 192, 192, 192, 255 };
    SDL_Color bronze = { 200, 127, 100, 255 };
    SDL_Color white = { 255, 255, 255, 255 };

    // Adicione os cabeçalhos das colunas para melhor organização
    int headerY = WINDOW_HEIGHT / 4;
    RenderTextAtPosition(renderer, LanguageManager::getText("rank").c_str(), WINDOW_WIDTH / 20 * 2, headerY, 30, textColor);
    RenderTextAtPosition(renderer, LanguageManager::getText("id").c_str(), WINDOW_WIDTH / 20 * 5, headerY, 30, textColor);
    RenderTextAtPosition(renderer, LanguageManager::getText("score").c_str(), WINDOW_WIDTH / 20 * 7.5, headerY, 30, textColor);
    RenderTextAtPosition(renderer, LanguageManager::getText("time").c_str(), WINDOW_WIDTH / 20 * 12.5, headerY, 30, textColor);

    const auto& scores = HighscoresManager::GetHighscores();
    int y = WINDOW_HEIGHT / 4 + 40; // Começa um pouco abaixo do cabeçalho
    int rank = 0;

    for (const auto& entry : scores) {
        // Renderiza cada coluna em uma posição fixa
        int fontsize;
        rank++;
        switch (rank) {
        case 1: 
            fontsize = 40;
            textColor = gold;
            break;
        case 2:
            fontsize = 34;
            textColor = silver;
            break;
        case 3:
            fontsize = 28;
            textColor = bronze;
            break;
        default:
            fontsize = 24;
            textColor = white;
            break;
        }
        RenderTextAtPosition(renderer, std::to_string(rank) + u8"°", WINDOW_WIDTH / 20 * 2, y, fontsize, textColor);
        RenderTextAtPosition(renderer, entry.name, WINDOW_WIDTH / 20 * 4, y, fontsize, textColor);
        RenderTextAtPosition(renderer, std::to_string(entry.score), WINDOW_WIDTH / 20 * 8, y, fontsize, textColor);
        RenderTextAtPosition(renderer, entry.timestamp, WINDOW_WIDTH / 20 * 11, y, fontsize, textColor);

        y += fontsize * 1.2; // Ajuste o espaçamento vertical entre as linhas
    }

    RenderCenteredText(renderer, LanguageManager::getText("pressEnterToReturn").c_str(), WINDOW_HEIGHT - 50, 24);
}

//Handle highscores screen events
void HandleHighscoresScreenEvents(SDL_Event& event, GameState& currentState) {
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER /*|| event.key.keysym.sym == SDLK_ESCAPE*/) {
            currentState = GameState::TITLE_SCREEN;
        }
    }
}

//Renders the name input screen
void RenderNameInputScreen(SDL_Renderer* renderer) {
    RenderCenteredText(renderer, LanguageManager::getText("highscoreMsg").c_str(), WINDOW_HEIGHT / 5, 50);
    RenderCenteredText(renderer, LanguageManager::getText("enterID").c_str(), WINDOW_HEIGHT / 3, 50);
    // Mostra o nome sendo digitado
    RenderCenteredText(renderer, (LanguageManager::getText("yourID") + ": " + playerName).c_str(), WINDOW_HEIGHT / 2, 40);

    RenderCenteredText(renderer, LanguageManager::getText("pressEnterToConfirm").c_str(), WINDOW_HEIGHT - 50, 24);
}

//Handle name input events
void HandleNameInputEvents(SDL_Event& event, GameState& currentState, bool& quit) {
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
            if (!playerName.empty()) {
                HighscoresManager::AddScore(playerName, score);
                currentState = GameState::HIGHSCORES_SCREEN;
            }
        }
        else if (event.key.keysym.sym == SDLK_BACKSPACE && !playerName.empty()) {
            playerName.pop_back();
        }
    }
    else if (event.type == SDL_TEXTINPUT) {
        if (playerName.length() < 7) { // Limite de 7 caracteres
            playerName += event.text.text;
        }
    }
}