// Main.cpp
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <deque>
#include <random>
#include <algorithm>

#include "Main.h"
#include "Title.h"
#include "Sound.h"
#include "Highscore.h"
#include "Settings.h"

using namespace std;

SDL_Texture* foodTexture = nullptr;

int SNAKE_COLOR_R = 255;
int SNAKE_COLOR_G = 255;
int SNAKE_COLOR_B = 255;
int SNAKE_COLOR_A = 255;
int GRID_BACKGROUND_COLOR_R = 34;
int GRID_BACKGROUND_COLOR_G = 139;
int GRID_BACKGROUND_COLOR_B = 34;
int GRID_BACKGROUND_COLOR_A = 255;

// Screen size variables
int WINDOW_WIDTH = MIN_WINDOW_WIDTH;
int WINDOW_HEIGHT = MIN_WINDOW_HEIGHT;
int CELL_SIZE = 40;
int GRID_OFFSET_X = (WINDOW_WIDTH - (GRID_SIZE * CELL_SIZE)) / 2;
int GRID_OFFSET_Y = (WINDOW_HEIGHT - (GRID_SIZE * CELL_SIZE)) / 2;

// Game state variables
deque<Point> snake;
Point food;
Direction currentDirection;
bool gameOver = false;
bool isPaused = false;
int score = 0;
Difficulty currentDifficulty;
int currentSpeed;
int foodEatenSinceLastSpeedIncrease;
GameState currentState = GameState::LANGUAGE_SELECT;
std::string playerName;
float scoreMultiplier = 1.0f;
static bool directionChangeLock = false;

// Game loop timing variables
const int TARGET_FPS = 60;
const int MS_PER_FRAME = 1000 / TARGET_FPS;
const int FIXED_TIMESTEP = 1000 / 60; // 60 updates per second for physics

// Internal game logic variables
static long long lastUpdateTime = 0;
static long long lastRestartTime = 0;
static long long lastAnimationTime = 0;
static long long lastBlinkTime = 0;
static bool showText = true;
static bool isCollisionAnimating = false;
static int collisionAnimationStep = 0;
static long long collisionAnimationStartTime = 0;
bool wallPassingMode = false;

// Timing variables for the new game loop
static long long accumulatedTime = 0;
static long long previousTime = 0;

// NEW: Snake movement timing
static long long timeSinceLastMove = 0;

extern SDL_Color textColor;

// Prototypes for internal functions
void PlaceFood();
void CheckSelfCollision();
void CheckFoodCollision();
long long GetTicks();
void HandleResize(int newWidth, int newHeight);

// Checks if the snake has collided with the game boundaries
void CheckBoundaryCollision() {
    Point& head = snake.front();

    if (wallPassingMode) {
        // Wall passing mode - teleport to opposite side
        if (head.x < 0) {
            head.x = GRID_SIZE - 1;
        }
        else if (head.x >= GRID_SIZE) {
            head.x = 0;
        }
        if (head.y < 0) {
            head.y = GRID_SIZE - 1;
        }
        else if (head.y >= GRID_SIZE) {
            head.y = 0;
        }
    }
    else {
        // Original behavior - game over when hitting walls
        if (head.x < 0 || head.x >= GRID_SIZE || head.y < 0 || head.y >= GRID_SIZE) {
            gameOver = true;
            isCollisionAnimating = true;
            collisionAnimationStep = 0;
            collisionAnimationStartTime = GetTicks();
            SoundSystem::PlaySound(SoundSystem::SoundType::GAME_OVER);
        }
    }
}

// Checks if the snake has collided with itself
void CheckSelfCollision() {
    Point head = snake.front();
    for (size_t i = 1; i < snake.size(); ++i) {
        if (head.x == snake[i].x && head.y == snake[i].y) {
            gameOver = true;
            isCollisionAnimating = true;
            collisionAnimationStep = 0;
            collisionAnimationStartTime = GetTicks();
            SoundSystem::PlaySound(SoundSystem::SoundType::GAME_OVER);
            break;
        }
    }
}

// Checks if the snake has eaten the food
void CheckFoodCollision() {
    if (snake.front() == food) {
        // Calculate score based on multiplier
        int pointsEarned = static_cast<int>(10 * scoreMultiplier);
        score += pointsEarned;

        foodEatenSinceLastSpeedIncrease++;
        SoundSystem::PlaySound(SoundSystem::SoundType::EAT_FOOD);

        if (foodEatenSinceLastSpeedIncrease >= 5) {
            currentSpeed = max(100, currentSpeed - 20); // Increase speed more gradually
            foodEatenSinceLastSpeedIncrease = 0;
            // Update multiplier based on speed
            UpdateScoreMultiplier();
        }

        PlaceFood();
    }
    else {
        snake.pop_back();
    }
}

void UpdateScoreMultiplier() {
    int baseSpeed = 0;
    switch (currentDifficulty) {
    case Difficulty::EASY: baseSpeed = 300; break;
    case Difficulty::NORMAL: baseSpeed = 250; break;
    case Difficulty::HARD: baseSpeed = 200; break;
    default: baseSpeed = 250; break;
    }

    int speedIncrease = baseSpeed - currentSpeed;
    int multiplierSteps = speedIncrease / 20; // Adjust for new speed reduction

    // Ensure multiplier is never less than initial value
    float minMultiplier = 1.0f;
    switch (currentDifficulty) {
    case Difficulty::EASY: minMultiplier = 1.0f; break;
    case Difficulty::NORMAL: minMultiplier = 1.5f; break;
    case Difficulty::HARD: minMultiplier = 2.0f; break;
    }

    if (scoreMultiplier < minMultiplier) {
        scoreMultiplier = minMultiplier;
    }

    scoreMultiplier = minMultiplier + (multiplierSteps * 0.10f);
}

// Places food at a random position on the grid that is not occupied by the snake
void PlaceFood() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, GRID_SIZE - 1);

    do {
        food.x = dis(gen);
        food.y = dis(gen);
    } while (find(snake.begin(), snake.end(), food) != snake.end());
}

// Function to load food texture
bool LoadFoodTexture(SDL_Renderer* renderer) {
    foodTexture = IMG_LoadTexture(renderer, GAME_FILES[0]);
    if (foodTexture == nullptr) {
        cerr << "Failed to load food texture: " << IMG_GetError() << endl;
        return false;
    }
    return true;
}

// Function to cleanup food texture
void CleanupFoodTexture() {
    if (foodTexture) {
        SDL_DestroyTexture(foodTexture);
        foodTexture = nullptr;
    }
}


void RenderCollisionAnimation(SDL_Renderer* renderer) {
    if (!isCollisionAnimating) return;

    long long currentTime = GetTicks();
    long long elapsedTime = currentTime - collisionAnimationStartTime;
    int totalAnimationTime = snake.size() * 50; // 50ms per segment

    if (elapsedTime >= totalAnimationTime) {
        isCollisionAnimating = false;
        return;
    }

    // Calculate how many segments should disappear (from head to tail)
    int segmentsToHide = elapsedTime / 50;
    segmentsToHide = std::max(0, std::min(segmentsToHide, (int)snake.size()));

    // Draw only segments that haven't disappeared yet (from tail to head)
    for (int i = snake.size() - 1; i >= segmentsToHide; --i) {
        const auto& segment = snake[i];
        SDL_Rect snakeRect = {
            GRID_OFFSET_X + segment.x * CELL_SIZE,
            GRID_OFFSET_Y + segment.y * CELL_SIZE,
            CELL_SIZE, CELL_SIZE
        };

        // Check if rectangle is valid before drawing
        if (snakeRect.w > 0 && snakeRect.h > 0) {
            // Make segments blink during animation
            if ((currentTime / 100) % 2 == 0) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red blinking
            }
            else {
                SDL_SetRenderDrawColor(renderer, SNAKE_COLOR);
            }
            SDL_RenderFillRect(renderer, &snakeRect);
        }
    }
}

// Renders the game screen
void RenderGameScreen(SDL_Renderer* renderer) {
    // Fill the background with the game background color
    SDL_SetRenderDrawColor(renderer, GAME_BACKGROUND_COLOR);
    SDL_RenderClear(renderer);

    // Calculate margins
    int horizontalMargin = 10;
    int verticalMargin = 10;

    // Draw grid background with margins
    SDL_SetRenderDrawColor(renderer, GRID_BACKGROUND_COLOR_R, GRID_BACKGROUND_COLOR_G, GRID_BACKGROUND_COLOR_B, GRID_BACKGROUND_COLOR_A);
    SDL_Rect gridBgRect = {
        GRID_OFFSET_X,
        GRID_OFFSET_Y,
        GRID_SIZE * CELL_SIZE,
        GRID_SIZE * CELL_SIZE
    };

    // Only draw if dimensions are valid
    if (gridBgRect.w > 0 && gridBgRect.h > 0) {
        SDL_RenderFillRect(renderer, &gridBgRect);
    }

    // --- Grid Border ---
    int borderSize = 3; // Border thickness in pixels
    SDL_Rect gridBorderRect = {
        GRID_OFFSET_X - borderSize,
        GRID_OFFSET_Y - borderSize,
        (GRID_SIZE * CELL_SIZE) + (borderSize * 2),
        (GRID_SIZE * CELL_SIZE) + (borderSize * 2)
    };

    // Only draw border if dimensions are valid and wall passing mode is disabled
    if (!wallPassingMode && gridBorderRect.w > 0 && gridBorderRect.h > 0) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black color for border
        SDL_RenderFillRect(renderer, &gridBorderRect); // Draw filled border

        // Redraw the grid background over the border (to create the margin effect)
        if (gridBgRect.w > 0 && gridBgRect.h > 0) {
            SDL_SetRenderDrawColor(renderer, GRID_BACKGROUND_COLOR_R, GRID_BACKGROUND_COLOR_G, GRID_BACKGROUND_COLOR_B, GRID_BACKGROUND_COLOR_A);
            SDL_RenderFillRect(renderer, &gridBgRect);
        }
    }

    // Draw the snake (only if game is not over OR if collision animation is still active)
    if (!gameOver || isCollisionAnimating) {
        // If collision animation is active, let the animation function handle the drawing
        if (isCollisionAnimating) {
            RenderCollisionAnimation(renderer);
        }
        else {
            // Normal snake drawing
            SDL_SetRenderDrawColor(renderer, SNAKE_COLOR_R, SNAKE_COLOR_G, SNAKE_COLOR_B, SNAKE_COLOR_A);
            for (const auto& segment : snake) {
                SDL_Rect segmentRect = {
                    GRID_OFFSET_X + segment.x * CELL_SIZE,
                    GRID_OFFSET_Y + segment.y * CELL_SIZE,
                    CELL_SIZE,
                    CELL_SIZE
                };

                // Check if rectangle is valid before drawing
                if (segmentRect.w > 0 && segmentRect.h > 0) {
                    SDL_RenderFillRect(renderer, &segmentRect);
                }
            }
        }
    }

    // Draw food (only if game is not over)
    if (!gameOver) {
        SDL_Rect foodRect = {
            GRID_OFFSET_X + food.x * CELL_SIZE,
            GRID_OFFSET_Y + food.y * CELL_SIZE,
            CELL_SIZE,
            CELL_SIZE
        };

        // Check if rectangle is valid before drawing
        if (foodRect.w > 0 && foodRect.h > 0) {
            if (foodTexture) {
                // Use loaded texture
                SDL_RenderCopy(renderer, foodTexture, NULL, &foodRect);
            }
            else {
                // Fallback to square if texture fails to load
                SDL_SetRenderDrawColor(renderer, FOOD_COLOR);
                SDL_RenderFillRect(renderer, &foodRect);
            }
        }
    }

    // Draw score text in the upper left corner
    std::string scoreText = LanguageManager::getText("score") + ": " + std::to_string(score);
    RenderTextAtPosition(renderer, scoreText, 10, 10, 24, textColor);

    // Draw debug information in the bottom right corner if DEBUG_MODE is enabled
#ifdef DEBUG_MODE
    std::string debugText = "Speed: " + std::to_string(currentSpeed) +
        " | Size: " + std::to_string(snake.size()) +
        " | Multiplier: " + std::to_string(scoreMultiplier) + "x";
    // Measure text size to position it correctly
    TTF_Font* font = TTF_OpenFont(TEXT_FONT, 20);
    int textWidth, textHeight;
    if (font) {
        TTF_SizeUTF8(font, debugText.c_str(), &textWidth, &textHeight);
        RenderTextAtPosition(renderer, debugText, WINDOW_WIDTH - textWidth - 10, WINDOW_HEIGHT - textHeight - 10, 20);
    }
#endif

    // If game is over, draw "Game Over" text
    if (gameOver) {
        RenderCenteredText(renderer, LanguageManager::getText("gameOver").c_str(), (WINDOW_HEIGHT / 2) - 50, 80);

        // Blinking "Press Space" text
        long long currentTime = GetTicks();
        if (currentTime > lastBlinkTime + BLINK_INTERVAL_MS) {
            showText = !showText;
            lastBlinkTime = currentTime;
        }

        if (showText) {
            RenderCenteredText(renderer, LanguageManager::getText("pressSpace").c_str(), WINDOW_HEIGHT / 2 + 100, 32);
        }

        // Draw collision animation if active
        if (isCollisionAnimating) {
            RenderCollisionAnimation(renderer);
        }
    }

    // If game is paused, draw "Paused" text
    if (isPaused) {
        RenderCenteredText(renderer, LanguageManager::getText("paused").c_str(), (WINDOW_HEIGHT / 2) - 50, 80);
    }
}

bool IsOppositeDirection(Direction dir1, Direction dir2) {
    return (dir1 == Direction::UP && dir2 == Direction::DOWN) ||
        (dir1 == Direction::DOWN && dir2 == Direction::UP) ||
        (dir1 == Direction::LEFT && dir2 == Direction::RIGHT) ||
        (dir1 == Direction::RIGHT && dir2 == Direction::LEFT);
}

bool IsValidDirection(Direction current, Direction newDir) {
    return !IsOppositeDirection(current, newDir);
}

// Handles events for the game screen
void HandleGameScreenEvents(SDL_Event& event, bool& quit) {
    if (event.type == SDL_KEYDOWN) {
        Direction newDirection = currentDirection;

        switch (event.key.keysym.sym) {
        case SDLK_UP:
        case SDLK_w:
            newDirection = Direction::UP;
            break;
        case SDLK_DOWN:
        case SDLK_s:
            newDirection = Direction::DOWN;
            break;
        case SDLK_LEFT:
        case SDLK_a:
            newDirection = Direction::LEFT;
            break;
        case SDLK_RIGHT:
        case SDLK_d:
            newDirection = Direction::RIGHT;
            break;
        case SDLK_SPACE:
            if (gameOver) {
                isCollisionAnimating = false;
                InitializeGame();
            }
            else {
                isPaused = !isPaused;
            }
            break;
        case SDLK_ESCAPE:
            currentState = GameState::TITLE_SCREEN;
            break;
        case SDLK_q:
            quit = true;
            break;
        }

        if (newDirection != currentDirection &&
            IsValidDirection(currentDirection, newDirection) &&
            !directionChangeLock) {

            currentDirection = newDirection;
            directionChangeLock = true; // Trava mudanças até o próximo movimento
        }
    }
}

// Fixed timestep game update
void FixedUpdateGame() {
    if (gameOver || isPaused) {
        return;
    }

    // Accumulate time since last move
    timeSinceLastMove += FIXED_TIMESTEP;

    // Only move snake when enough time has passed based on current speed
    if (timeSinceLastMove >= currentSpeed) {
        timeSinceLastMove = 0;

        // LIBERA o lock de mudança de direção a cada movimento
        directionChangeLock = false;

        Point newHead = snake.front();
        switch (currentDirection) {
        case Direction::UP:
            newHead.y--;
            break;
        case Direction::DOWN:
            newHead.y++;
            break;
        case Direction::LEFT:
            newHead.x--;
            break;
        case Direction::RIGHT:
            newHead.x++;
            break;
        }
        snake.push_front(newHead);

        CheckBoundaryCollision();
        CheckFoodCollision();
        CheckSelfCollision();
    }
}

// Initializes the game state
void InitializeGame() {
    snake.clear();
    Point initialHead = { GRID_SIZE / 2, GRID_SIZE / 2 };
    Point initialBody;
    currentDirection = Direction::RIGHT;

    initialBody.x = initialHead.x - 1;
    initialBody.y = initialHead.y;

    snake.push_front(initialHead);
    snake.push_back(initialBody);

    // Reset movement timer
    timeSinceLastMove = 0;
    gameOver = false;
    isPaused = false;
    isCollisionAnimating = false;
    collisionAnimationStep = 0;
    score = 0;
    foodEatenSinceLastSpeedIncrease = 0;

    directionChangeLock = false;

    switch (currentDifficulty) {
    case Difficulty::EASY:
        scoreMultiplier = 1.0f;
        currentSpeed = 300;
        break;
    case Difficulty::NORMAL:
        scoreMultiplier = 1.5f;
        currentSpeed = 250;
        break;
    case Difficulty::HARD:
        scoreMultiplier = 2.0f;
        currentSpeed = 200;
        break;
    }

    PlaceFood();
}

// Restarts the game with the selected difficulty
void NewGame(Difficulty difficulty) {
    currentDifficulty = difficulty;
    switch (currentDifficulty) {
    case Difficulty::EASY:
        currentSpeed = 300;
        scoreMultiplier = 1.0f;
        break;
    case Difficulty::NORMAL:
        currentSpeed = 250;
        scoreMultiplier = 1.5f;
        break;
    case Difficulty::HARD:
        currentSpeed = 200;
        scoreMultiplier = 2.0f;
        break;
    }
    InitializeGame();
}

// Function to handle window resize events
void HandleResize(int newWidth, int newHeight) {
    // Ensure minimum window size
    WINDOW_WIDTH = max(newWidth, MIN_WINDOW_WIDTH);
    WINDOW_HEIGHT = max(newHeight, MIN_WINDOW_HEIGHT);

    // Calculate available space for the grid (subtracting margins)
    int horizontalMargin = 10; // 10px on each side
    int verticalMargin = 10;   // 10px on top and bottom

    int availableWidth = WINDOW_WIDTH - (2 * horizontalMargin);
    int availableHeight = WINDOW_HEIGHT - (2 * verticalMargin);

    // Calculate cell size based on the smaller dimension of available space
    int maxCellSizeFromWidth = availableWidth / GRID_SIZE;
    int maxCellSizeFromHeight = availableHeight / GRID_SIZE;
    CELL_SIZE = max(1, min(maxCellSizeFromWidth, maxCellSizeFromHeight));

    // Center the grid with margins
    GRID_OFFSET_X = horizontalMargin + (availableWidth - (GRID_SIZE * CELL_SIZE)) / 2;
    GRID_OFFSET_Y = verticalMargin + (availableHeight - (GRID_SIZE * CELL_SIZE)) / 2;
}

// Main game loop
int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) != 0) {
        cerr << "SDL_Init Error: " << SDL_GetError() << endl;
        return 1;
    }
    if (TTF_Init() != 0) {
        cerr << "TTF_Init Error: " << TTF_GetError() << endl;
        SDL_Quit();
        return 1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        cerr << "IMG_Init Error: " << IMG_GetError() << endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    if (!SoundSystem::Initialize()) {
        cerr << "SoundSystem initialization failed." << endl;
    }
    SoundSystem::LoadSounds();
    HighscoresManager::LoadHighscores();

    SDL_Window* window = SDL_CreateWindow(LanguageManager::getText("gameTitle").c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        cerr << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
        SoundSystem::Shutdown();
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        SoundSystem::Shutdown();
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    if (!LoadFoodTexture(renderer)) {
        cerr << "Warning: Using fallback square for food" << endl;
    }

    if (!LoadTitleTexture(renderer)) {
        std::cerr << "Warning: Could not load title image. Falling back to text." << std::endl;
        // Não define success = false, pois há um fallback de texto
    }

    LoadVolumeIcons(renderer);
    int initialWidth, initialHeight;
    SDL_GetWindowSize(window, &initialWidth, &initialHeight);
    HandleResize(initialWidth, initialHeight);
    InitializeGame();

    bool quit = false;
    SDL_Event event;

    // Initialize timing variables for the new game loop
    previousTime = GetTicks();
    accumulatedTime = 0;
    timeSinceLastMove = 0;

    while (!quit) {
        long long currentTime = GetTicks();
        long long deltaTime = currentTime - previousTime;
        previousTime = currentTime;

        accumulatedTime += deltaTime;

        // Process events every frame for maximum responsiveness
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    HandleResize(event.window.data1, event.window.data2);
                }
                else if (event.window.event == SDL_WINDOWEVENT_SHOWN) {
                    // Ensure proper sizing when window is first shown
                    int width, height;
                    SDL_GetWindowSize(window, &width, &height);
                    HandleResize(width, height);
                }
            }
            else {
                switch (currentState) {
                case GameState::LANGUAGE_SELECT:
                    HandleLanguageScreenEvents(event, currentState, quit);
                    break;
                case GameState::TITLE_SCREEN:
                    HandleTitleScreenEvents(event, currentState, quit);
                    break;
                case GameState::DIFFICULTY_SCREEN:
                    HandleDifficultyScreenEvents(event, currentState);
                    break;
                case GameState::HIGHSCORES_SCREEN:
                    HandleHighscoresScreenEvents(event, currentState);
                    break;
                case GameState::NAME_INPUT_SCREEN:
                    HandleNameInputEvents(event, currentState, quit);
                    break;
                case GameState::GAME_SCREEN:
                    HandleGameScreenEvents(event, quit);
                    break;
                case GameState::SETTINGS_SCREEN:
                    HandleSettingsScreenEvents(event, currentState, quit);
                    break;
                }
            }
        }

        if (currentState == GameState::NAME_INPUT_SCREEN) {
            SDL_StartTextInput();
        }
        else {
            SDL_StopTextInput();
        }

        // Fixed timestep game updates for consistent gameplay
        while (accumulatedTime >= FIXED_TIMESTEP) {
            if (currentState == GameState::GAME_SCREEN && !gameOver && !isPaused) {
                FixedUpdateGame();
            }
            accumulatedTime -= FIXED_TIMESTEP;
        }

        if (gameOver && !isCollisionAnimating) {
            if (HighscoresManager::IsHighscore(score)) {
                currentState = GameState::NAME_INPUT_SCREEN;
                gameOver = false; // Reset to avoid loop
                playerName = ""; // Clear name
            }
        }

        // Render everything
        SDL_SetRenderDrawColor(renderer, GAME_BACKGROUND_COLOR);
        SDL_RenderClear(renderer);

        switch (currentState) {
        case GameState::LANGUAGE_SELECT:
            RenderLanguageScreen(renderer);
            break;
        case GameState::TITLE_SCREEN:
            RenderTitleScreen(renderer);
            break;
        case GameState::DIFFICULTY_SCREEN:
            RenderDifficultyScreen(renderer);
            break;
        case GameState::HIGHSCORES_SCREEN:
            RenderHighscoresScreen(renderer);
            break;
        case GameState::NAME_INPUT_SCREEN:
            RenderNameInputScreen(renderer);
            break;
        case GameState::GAME_SCREEN:
            RenderGameScreen(renderer);
            break;
        case GameState::SETTINGS_SCREEN:
            RenderSettingsScreen(renderer);
            break;
        }

        SDL_RenderPresent(renderer);

        // Frame rate control for consistent performance
        long long frameTime = GetTicks() - currentTime;
        if (frameTime < MS_PER_FRAME) {
            SDL_Delay(MS_PER_FRAME - frameTime);
        }
    }

    CleanupFoodTexture();
    CleanupTitleTexture();
    CleanupVolumeIcons();
    HighscoresManager::SaveHighscores();
    SoundSystem::Shutdown();
    IMG_Quit();
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

long long GetTicks() {
    auto duration = chrono::steady_clock::now().time_since_epoch();
    return chrono::duration_cast<chrono::milliseconds>(duration).count();
}