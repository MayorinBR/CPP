#include "Highscore.h"
#include <fstream>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <ctime>

std::vector<ScoreEntry> HighscoresManager::highscores;
const std::string HighscoresManager::HIGHSCORE_FILE = "highscores.dat";
static const int MAX_HIGHSCORES = 10;

void HighscoresManager::LoadHighscores() {
    highscores.clear();
    std::ifstream file(HIGHSCORE_FILE);
    if (!file.is_open()) {
        std::cerr << "Highscores file not found. Creating a new one on save." << std::endl;
        return;
    }

    ScoreEntry entry;
    while (file >> entry.name >> entry.score) {
        std::getline(file, entry.timestamp);
        if (!entry.timestamp.empty() && entry.timestamp[0] == ' ') {
            entry.timestamp.erase(0, 1);
        }
        highscores.push_back(entry);
    }
    file.close();
}

void HighscoresManager::SaveHighscores() {
    std::ofstream file(HIGHSCORE_FILE);
    if (!file.is_open()) {
        std::cerr << "Failed to save highscores file." << std::endl;
        return;
    }

    for (const auto& entry : highscores) {
        file << entry.name << " " << entry.score << " " << entry.timestamp << std::endl;
    }
    file.close();
}


void HighscoresManager::AddScore(const std::string& name, int score) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    struct tm timeinfo;
    char buf[100];

    if (localtime_s(&timeinfo, &now_c) == 0) {
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
        std::string timestamp = buf;
        highscores.push_back({ name, score, timestamp });
    }
    else {
        std::cerr << "Erro ao converter o tempo." << std::endl;
        highscores.push_back({ name, score, "N/A" });
    }

    std::sort(highscores.begin(), highscores.end(), [](const ScoreEntry& a, const ScoreEntry& b) {
        return a.score > b.score;
        });

    if (static_cast<int>(highscores.size()) > MAX_HIGHSCORES) {
        highscores.resize(MAX_HIGHSCORES);
    }
}

bool HighscoresManager::IsHighscore(int score) {
    if (highscores.size() < MAX_HIGHSCORES) {
        return true;
    }
    return score > highscores.back().score;
}

const std::vector<ScoreEntry>& HighscoresManager::GetHighscores() {
    return highscores;
}