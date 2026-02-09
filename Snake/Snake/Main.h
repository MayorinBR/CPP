#pragma once
#include <SDL.h>
#include <vector>
#include <string>
#include <deque>

// Minimum window size for a playable experience
#define MIN_WINDOW_WIDTH 800
#define MIN_WINDOW_HEIGHT 600

// Comment/uncomment to deactivate/activate
//#define DEBUG_MODE

// Game constants
#define GRID_SIZE 15        // 15x15 grid
#define RESTART_TIME 3      // Time in seconds before restart
#define BASE_FONT_SIZE 24
#define TEXT_FONT "NotoSansJP-Variable.ttf"
#define ASPECT_RATIO (float)MIN_WINDOW_WIDTH / (float)MIN_WINDOW_HEIGHT
#define FLAG_SIZE_FACTOR 5
#define BLINK_INTERVAL_MS 500 // 0.5 seconds para piscar o texto

// Colors
#define GAME_BACKGROUND_COLOR 125, 125, 125, 255    // Black
#define GRID_BACKGROUND_COLOR 34, 139, 34, 255 // Green for the grid
#define SNAKE_COLOR 255, 255, 255, 255        // White for the snake
#define FOOD_COLOR 125, 125, 125, 255      // Red
#define GRID_COLOR 0, 100, 0, 255      // Darker green for lines
#define TEXT_COLOR 255, 255, 255, 255  // White

extern SDL_Texture* foodTexture;

extern int SNAKE_COLOR_R, SNAKE_COLOR_G, SNAKE_COLOR_B, SNAKE_COLOR_A;
extern int GRID_BACKGROUND_COLOR_R, GRID_BACKGROUND_COLOR_G, GRID_BACKGROUND_COLOR_B, GRID_BACKGROUND_COLOR_A;
extern bool wallPassingMode;

// Enumerations for game state, direction, language, and difficulty
enum class GameState {
    LANGUAGE_SELECT, // New state for language selection
    TITLE_SCREEN,
    DIFFICULTY_SCREEN,
    HIGHSCORES_SCREEN,
    NAME_INPUT_SCREEN,
    GAME_SCREEN,
    SETTINGS_SCREEN
};

enum class Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

enum class Language {
    ENGLISH,
    JAPANESE,
    PORTUGUESE,
    LANGUAGE_COUNT // The total number of languages
};

bool IsOppositeDirection(Direction dir1, Direction dir2);
bool IsValidDirection(Direction current, Direction newDir);

// Represents a point on the grid
struct Point {
    int x, y;
    bool operator==(const Point& other) const {
        return x == other.x && other.y == y;
    }
};

enum class Difficulty {
    EASY,
    NORMAL,
    HARD
};

static constexpr const char* GAME_FILES[] = {
    "assets/Food.png",
};

// Screen size variables
extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;
extern int CELL_SIZE;
extern int GRID_OFFSET_X;
extern int GRID_OFFSET_Y;

// Game state variables
extern std::deque<Point> snake;
extern Point food;
extern Direction currentDirection;
extern bool gameOver;
extern bool isPaused;
extern bool directionChanged;
extern int score;
extern Difficulty currentDifficulty;
extern int currentSpeed;
extern int foodEatenSinceLastSpeedIncrease;
extern GameState currentState;
extern Language currentLanguage;
extern std::string playerName;
extern float scoreMultiplier;

// Function prototypes
void InitializeGame();
void NewGame(Difficulty difficulty);
void HandleGameScreenEvents(SDL_Event& event, bool& quit);
void RenderGameScreen(SDL_Renderer* renderer);
void NewGame(Difficulty difficulty);
void HandleResize(int newWidth, int newHeight); void RenderTextAtPosition(SDL_Renderer* renderer, const std::string& text, int x, int y, int fontSize, SDL_Color color = { 255, 255, 255, 255 });
void RenderCenteredText(SDL_Renderer* renderer, const char* text, int y, int fontSize);
void UpdateScoreMultiplier();
void CheckBoundaryCollision();
bool IsOppositeDirection(Direction dir1, Direction dir2);
bool IsValidDirection(Direction current, Direction newDir);