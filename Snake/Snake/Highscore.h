#pragma once
#include <vector>
#include <string>

// Estrutura para armazenar o nome do jogador, a pontuação e o timestamp
struct ScoreEntry {
    std::string name;
    int score = 0; // Inicialização direta
    std::string timestamp;

    // Construtor para inicializar todos os membros
    ScoreEntry(const std::string& n, int s, const std::string& ts)
        : name(n), score(s), timestamp(ts) {
    }

    ScoreEntry() : score(0) {}
};

// Classe para gerenciar highscores
class HighscoresManager {
public:
    static void SaveHighscores();
    static void LoadHighscores();
    static void AddScore(const std::string& name, int score);
    static bool IsHighscore(int score);
    static const std::vector<ScoreEntry>& GetHighscores();

private:
    static std::vector<ScoreEntry> highscores;
    static const std::string HIGHSCORE_FILE;
    static const int MAX_HIGHSCORES = 10;
};