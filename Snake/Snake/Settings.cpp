#include "Settings.h"
#include "Title.h"
#include "Sound.h"
#include "LanguageManager.h"
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <fstream>

// Variáveis
extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;
extern int SNAKE_COLOR_R, SNAKE_COLOR_G, SNAKE_COLOR_B, SNAKE_COLOR_A;
extern int GRID_BACKGROUND_COLOR_R, GRID_BACKGROUND_COLOR_G, GRID_BACKGROUND_COLOR_B, GRID_BACKGROUND_COLOR_A;

// Opções
static int currentSettingsSelection = 0;
static int snakeColorIndex = 0;         //White
static int boardColorIndex = 2;         //Green
static int currentBgmSelection = 0;

const int SETTINGS_COUNT = 6;

extern SDL_Color textColor;

// Cores predefinidas
static const std::vector<Color> predefinedColors = {
    {255, 255, 255, 255}, // Branco
    {0, 0, 0, 255},       // Preto
    {34, 139, 34, 255},   // Verde
    {255, 0, 0, 255},     // Vermelho
    {255, 255, 0, 255},   // Amarelo
    {0, 0, 255, 255},     // Azul
    {128, 0, 128, 255},   // Roxo
    {255, 192, 203, 255}, // Rosa
};

// Nova variável para armazenar os nomes dos BGMs
static const std::vector<std::string> bgmNames = { "BGM1", "BGM2", "BGM3" };

// Texturas para os ícones de som (agora definidas aqui)
SDL_Texture* muteIconTexture = nullptr;
SDL_Texture* unmuteIconTexture = nullptr;

Color adjustContrastColor(const Color& color, int amount) {
    // Calcula a "luminosidade" da cor para decidir se deve clarear ou escurecer
    int brightness = (color.r + color.g + color.b) / 3;

    if (brightness > 128) { // A cor é clara, então vamos escurecer
        return {
            std::max(0, color.r - amount),
            std::max(0, color.g - amount),
            std::max(0, color.b - amount),
            color.a
        };
    }
    else { // A cor é escura, então vamos clarear
        return {
            std::min(255, color.r + amount),
            std::min(255, color.g + amount),
            std::min(255, color.b + amount),
            color.a
        };
    }
}

// Funções auxiliares para as cores
void applyColor(const std::string& key, const Color& color) {
    Color finalGridColor = color;

    if (key == "grid") {
        Color snakeColor = { SNAKE_COLOR_R, SNAKE_COLOR_G, SNAKE_COLOR_B, SNAKE_COLOR_A };

        if (finalGridColor.r == snakeColor.r && 
            finalGridColor.g == snakeColor.g &&
            finalGridColor.b == snakeColor.b) {
            
            finalGridColor = adjustContrastColor(finalGridColor, 100); 
        }
    }

    // Aplique as cores
    if (key == "snake") {
        SNAKE_COLOR_R = color.r;
        SNAKE_COLOR_G = color.g;
        SNAKE_COLOR_B = color.b;
        SNAKE_COLOR_A = color.a;
    }
    else if (key == "grid") {
        GRID_BACKGROUND_COLOR_R = finalGridColor.r;
        GRID_BACKGROUND_COLOR_G = finalGridColor.g;
        GRID_BACKGROUND_COLOR_B = finalGridColor.b;
        GRID_BACKGROUND_COLOR_A = finalGridColor.a;
    }
}

void RenderColorRect(SDL_Renderer* renderer, const Color& color, int x, int y) {
    SDL_Rect rect = { x, y, 80, 20 };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Resetar para branco
    SDL_RenderDrawRect(renderer, &rect); // Adiciona uma borda
}

// Funções para renderizar a barra de volume
void RenderVolumeBar(SDL_Renderer* renderer, int x, int y, int volume, int barWidth, int barHeight) {
    SDL_Color lightGreen = { 144, 238, 144, 255 }; // Light Green

    // Desenha o fundo da barra
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_Rect backgroundRect = { x, y, barWidth, barHeight };
    SDL_RenderFillRect(renderer, &backgroundRect);

    // Desenha a barra de volume
    int barFillWidth = static_cast<int>((static_cast<double>(volume) / MIX_MAX_VOLUME) * barWidth);
    SDL_SetRenderDrawColor(renderer, lightGreen.r, lightGreen.g, lightGreen.b, lightGreen.a);
    SDL_Rect fillRect = { x, y, barFillWidth, barHeight };
    SDL_RenderFillRect(renderer, &fillRect);
}

void LoadVolumeIcons(SDL_Renderer* renderer) {
    // Limpa texturas existentes primeiro
    CleanupVolumeIcons();

    if (!renderer) {
        std::cerr << "ERRO: Renderer é nulo ao carregar ícones" << std::endl;
        return;
    }

    // Lista de possíveis caminhos para testar
    std::vector<std::string> mutePath = {
        "assets/mute.png"
    };

    std::vector<std::string> unmutePath = {
        "assets/unmute.png"
    };

    //std::cout << "=== TENTANDO CARREGAR ÍCONE MUTE ===" << std::endl;
    bool muteLoaded = false;
    for (const auto& path : mutePath) {
        //std::cout << "Tentando: " << path << std::endl;
        muteIconTexture = IMG_LoadTexture(renderer, path.c_str());
        if (muteIconTexture) {
            //std::cout << "✓ Ícone mute carregado de: " << path << std::endl;
            muteLoaded = true;

            // Verifica informações da textura
            int width, height;
            SDL_QueryTexture(muteIconTexture, NULL, NULL, &width, &height);
            //std::cout << "  Dimensões: " << width << "x" << height << " pixels" << std::endl;
            break;
        }
        else {
            //std::cout << "✗ Falha: " << IMG_GetError() << std::endl;
        }
    }

    if (!muteLoaded) {
        //std::cout << "Criando ícone fallback para mute..." << std::endl;
        SDL_Surface* surface = SDL_CreateRGBSurface(0, 32, 32, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
        if (surface) {
            SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, 255, 0, 0, 255)); // Vermelho
            muteIconTexture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            //std::cout << "Ícone fallback mute criado" << std::endl;
        }
    }

    //std::cout << "=== TENTANDO CARREGAR ÍCONE UNMUTE ===" << std::endl;
    bool unmuteLoaded = false;
    for (const auto& path : unmutePath) {
        //std::cout << "Tentando: " << path << std::endl;
        unmuteIconTexture = IMG_LoadTexture(renderer, path.c_str());
        if (unmuteIconTexture) {
            //std::cout << "✓ Ícone unmute carregado de: " << path << std::endl;
            unmuteLoaded = true;

            // Verifica informações da textura
            int width, height;
            SDL_QueryTexture(unmuteIconTexture, NULL, NULL, &width, &height);
            //std::cout << "  Dimensões: " << width << "x" << height << " pixels" << std::endl;
            break;
        }
        else {
            //std::cout << "✗ Falha: " << IMG_GetError() << std::endl;
        }
    }

    if (!unmuteLoaded) {
        std::cout << "Criando ícone fallback para unmute..." << std::endl;
        SDL_Surface* surface = SDL_CreateRGBSurface(0, 32, 32, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
        if (surface) {
            SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, 0, 255, 0, 255)); // Verde
            unmuteIconTexture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            //std::cout << "Ícone fallback unmute criado" << std::endl;
        }
    }
}
void CleanupVolumeIcons() {
    if (muteIconTexture) {
        SDL_DestroyTexture(muteIconTexture);
        muteIconTexture = nullptr;
    }
    if (unmuteIconTexture) {
        SDL_DestroyTexture(unmuteIconTexture);
        unmuteIconTexture = nullptr;
    }
}

void RenderVolumeIcon(SDL_Renderer* renderer, int x, int y, bool isMuted, int volume) {
    SDL_Texture* textureToRender = nullptr;
    if (isMuted || volume == 0) {
        textureToRender = muteIconTexture;
    }
    else {
        textureToRender = unmuteIconTexture;
    }

    if (textureToRender) {
        SDL_Rect iconRect = { x, y, 32, 32 };

        // Verifica o tamanho da textura
        int width, height;
        SDL_QueryTexture(textureToRender, NULL, NULL, &width, &height);
        //std::cout << "Renderizando ícone: " << width << "x" << height << std::endl;

        SDL_RenderCopy(renderer, textureToRender, NULL, &iconRect);
    }
    else {
        std::cout << "Textura não disponível, usando fallback" << std::endl;
        // Fallback colorido baseado no estado
        SDL_Rect iconRect = { x, y, 32, 32 };
        if (isMuted || volume == 0) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Vermelho para mudo
        }
        else {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Verde para som
        }
        SDL_RenderFillRect(renderer, &iconRect);

        // Borda branca
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &iconRect);
    }
}

void RenderCheckbox(SDL_Renderer* renderer, int x, int y, bool checked) {
    SDL_Rect checkboxRect = { x, y, 30, 30 };

    // Fundo da checkbox
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &checkboxRect);

    // Borda
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &checkboxRect);

    // Marcação se estiver checked
    if (checked) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_Rect checkRect = { x + 5, y + 5, 20, 20 };
        SDL_RenderFillRect(renderer, &checkRect);
    }
}

void RenderSettingsScreen(SDL_Renderer* renderer) {
    int yOffset = WINDOW_HEIGHT / 12;
    int spacing = 60;
    int fontSize = 32;

    RenderCenteredText(renderer, LanguageManager::getText("settings").c_str(), yOffset, 60);

    // Opções
    std::string sfxVolumeText = LanguageManager::getText("volumeSFX") + ": ";
    RenderTextAtPosition(renderer, sfxVolumeText, WINDOW_WIDTH / 6, yOffset + spacing * 2, fontSize, textColor);
    RenderVolumeBar(renderer, WINDOW_WIDTH / 2, yOffset + spacing * 2 + 20, SoundSystem::currentVolume, 200, 20);
    RenderVolumeIcon(renderer, WINDOW_WIDTH / 2 + 210, yOffset + spacing * 2 + 10,
        SoundSystem::isMuted, SoundSystem::currentVolume);

    std::string bgmVolumeText = LanguageManager::getText("volumeBGM") + ": ";
    RenderTextAtPosition(renderer, bgmVolumeText, WINDOW_WIDTH / 6, yOffset + spacing * 3, fontSize, textColor);
    RenderVolumeBar(renderer, WINDOW_WIDTH / 2, yOffset + spacing * 3 + 20, SoundSystem::bgmVolume, 200, 20);
    RenderVolumeIcon(renderer, WINDOW_WIDTH / 2 + 210, yOffset + spacing * 3 + 10,
        SoundSystem::isBgmMuted, SoundSystem::bgmVolume);

    std::string bgmSelectText = LanguageManager::getText("bgmSelect") + ": ";
    RenderTextAtPosition(renderer, bgmSelectText, WINDOW_WIDTH / 6, yOffset + spacing * 4, fontSize, textColor);
    std::string bgmName = bgmNames[static_cast<size_t>(currentBgmSelection)];
    RenderTextAtPosition(renderer, bgmName.c_str(), WINDOW_WIDTH / 2, yOffset + spacing * 4, fontSize, textColor);

    std::string snakeColorText = LanguageManager::getText("snakeColor") + ": ";
    RenderTextAtPosition(renderer, snakeColorText, WINDOW_WIDTH / 6, yOffset + spacing * 5, fontSize, textColor);
    RenderColorRect(renderer, predefinedColors[static_cast<size_t>(snakeColorIndex)], WINDOW_WIDTH * 11 / 20, yOffset + spacing * 5 + 20);

    std::string boardColorText = LanguageManager::getText("boardColor") + ": ";
    RenderTextAtPosition(renderer, boardColorText, WINDOW_WIDTH / 6, yOffset + spacing * 6, fontSize, textColor);
    RenderColorRect(renderer, predefinedColors[static_cast<size_t>(boardColorIndex)], WINDOW_WIDTH * 11 / 20, yOffset + spacing * 6 + 20);

    std::string wallPassingText = LanguageManager::getText("wallPassing") + ": ";
    RenderTextAtPosition(renderer, wallPassingText, WINDOW_WIDTH / 6, yOffset + spacing * 7, fontSize, textColor);
    RenderCheckbox(renderer, WINDOW_WIDTH * 11/20, yOffset + spacing * 7 + 10, wallPassingMode);

    // Render indicator of current selection
    RenderTextAtPosition(renderer, ">", WINDOW_WIDTH / 6 - 25, yOffset + spacing * (currentSettingsSelection + 2), fontSize, textColor);
    RenderCenteredText(renderer, LanguageManager::getText("pressEnterToReturn").c_str(), WINDOW_HEIGHT - 50, 24);
}

void HandleSettingsScreenEvents(SDL_Event& event, GameState& currentState, bool& quit) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        case SDLK_UP:
            currentSettingsSelection = (currentSettingsSelection - 1 + SETTINGS_COUNT) % SETTINGS_COUNT;
            SoundSystem::PlaySound(SoundSystem::SoundType::MENU_SELECT);
            break;
        case SDLK_DOWN:
            currentSettingsSelection = (currentSettingsSelection + 1) % SETTINGS_COUNT;
            SoundSystem::PlaySound(SoundSystem::SoundType::MENU_SELECT);
            break;
        case SDLK_LEFT:
            if (currentSettingsSelection == 0) { // SFX Volume
                SoundSystem::SetVolume(SoundSystem::currentVolume - 10);
            }
            else if (currentSettingsSelection == 1) { // BGM Volume
                SoundSystem::SetBgmVolume(SoundSystem::bgmVolume - 10);
            }
            else if (currentSettingsSelection == 2) { // BGM Selection
                currentBgmSelection = (currentBgmSelection - 1 + static_cast<int>(bgmNames.size())) % static_cast<int>(bgmNames.size());
                SoundSystem::PlayBgm(currentBgmSelection);
            }
            else if (currentSettingsSelection == 3) { // Snake Color
                snakeColorIndex = (snakeColorIndex - 1 + static_cast<int>(predefinedColors.size())) % static_cast<int>(predefinedColors.size());
            }
            else if (currentSettingsSelection == 4) { // Grid Color
                boardColorIndex = (boardColorIndex - 1 + static_cast<int>(predefinedColors.size())) % static_cast<int>(predefinedColors.size());
            }
            else if (currentSettingsSelection == 5) { // Wall Passing Mode
                wallPassingMode = !wallPassingMode;
            }
            SoundSystem::PlaySound(SoundSystem::SoundType::MENU_SELECT);
            break;
        case SDLK_RIGHT:
            if (currentSettingsSelection == 0) { // SFX Volume
                SoundSystem::SetVolume(SoundSystem::currentVolume + 10);
            }
            else if (currentSettingsSelection == 1) { // BGM Volume
                SoundSystem::SetBgmVolume(SoundSystem::bgmVolume + 10);
            }
            else if (currentSettingsSelection == 2) { // BGM Selection
                currentBgmSelection = (currentBgmSelection + 1) % static_cast<int>(bgmNames.size());
                SoundSystem::PlayBgm(currentBgmSelection);
            }
            else if (currentSettingsSelection == 3) { // Snake Color

                snakeColorIndex = (snakeColorIndex + 1) % static_cast<int>(predefinedColors.size());
            }
            else if (currentSettingsSelection == 4) { // Grid Color
                boardColorIndex = (boardColorIndex + 1) % static_cast<int>(predefinedColors.size());
            }
            else if (currentSettingsSelection == 5) { // Wall Passing Mode
                wallPassingMode = !wallPassingMode;
            }
            SoundSystem::PlaySound(SoundSystem::SoundType::MENU_SELECT);
            break;
        case SDLK_SPACE: // Toggle Mute
            if (currentSettingsSelection == 0) {
                SoundSystem::ToggleMute();
            }
            else if (currentSettingsSelection == 1) {
                SoundSystem::ToggleBgmMute();
            }
            else if (currentSettingsSelection == 5) { // Wall Passing Mode
                wallPassingMode = !wallPassingMode;
            }
            break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            applyColor("snake", predefinedColors[static_cast<size_t>(snakeColorIndex)]);
            applyColor("grid", predefinedColors[static_cast<size_t>(boardColorIndex)]);
            currentState = GameState::TITLE_SCREEN;
            break;
        /*case SDLK_ESCAPE:
            currentState = GameState::TITLE_SCREEN;
            break;*/
        }
    }
}

