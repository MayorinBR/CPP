#include "LanguageManager.h"
#include <map>
#include <string>
#include <vector>

Language currentLanguage = Language::ENGLISH;

// Static map that stores the strings
static std::map<Language, std::map<std::string, std::string>> allStrings = {
    { Language::PORTUGUESE, {
        {"gameTitle", u8"Jogo da Cobrinha"},
        {"pressSpace", u8"Pressione espaço para começar"},
        {"pressQuit", u8"Pressione ESC/Q para sair"},
        {"easy", u8"Fácil"},
        {"normal", u8"Normal"},
        {"hard", u8"Difícil"},
        {"difficultySelect", u8"Selecione a Dificuldade"},
        {"gameOver", u8"Fim de Jogo"},
        {"score", u8"Pontuação"},
        {"english", u8"Inglês"},
        {"japanese", u8"Japonês"},
        {"portuguese", u8"Português"},
        {"pressEnter", u8"Pressione Enter para continuar"},
        {"languageSelect", u8"Selecione o Idioma"},
        {"highscores", u8"Highscores"},
        {"highscoresTitle", u8"Melhores Pontuações"},
        {"pressEnterToReturn", u8"Pressione Enter para voltar"},
        {"highscoreMsg", u8"Você fez um Highscore!"},
        {"enterID", u8"Digite seu ID:"},
        {"yourID", u8"ID"},
        {"pressEnterToConfirm", u8"Pressione Enter para confirmar"},
        {"paused", u8"Pausado"},
        {"rank", u8"Rank"},
        {"id", u8"ID"},
        {"time", u8"Data e Hora"},
        {"startGame", u8"Iniciar Jogo"},
        {"quitGame", u8"Encerrar Jogo"},
        {"settings", u8"Configurações"},
        {"volumeSFX", u8"Volume do SFX"},
        {"volumeBGM", u8"Volume do BGM"},
        {"bgmSelect", u8"Seletor de BGM"},
        {"snakeColor", u8"Cor da Cobra"},
        {"boardColor", u8"Cor da Mapa"},
        {"wallPassing", u8"Atravessar Paredes"},
    }},
    { Language::ENGLISH, {
        {"gameTitle", u8"Snake Game"},
        {"pressSpace", u8"Press SPACE to start"},
        {"pressQuit", u8"Press ESC/Q to quit"},
        {"easy", u8"Easy"},
        {"normal", u8"Normal"},
        {"hard", u8"Hard"},
        {"difficultySelect", u8"Select Difficulty"},
        {"gameOver", u8"Game Over"},
        {"score", u8"Score"},
        {"english", u8"English"},
        {"japanese", u8"Japanese"},
        {"portuguese", u8"Portuguese"},
        {"pressEnter", u8"Press Enter to continue"},
        {"languageSelect", u8"Select Language"},
        {"highscores", u8"Highscores"},
        {"highscoresTitle", u8"Top Scores"},
        {"pressEnterToReturn", u8"Press Enter to return"},
        {"highscoreMsg", u8"You got a highscore!"},
        {"enterID", u8" Enter your ID:"},
        {"yourID", u8"Your name"},
        {"pressEnterToConfirm", u8"Press Enter to confirm"},
        {"paused", u8"Paused"},
        {"rank", u8"Rank"},
        {"id", u8"ID"},
        {"time", u8"Date and Time"},
        {"startGame", u8"Start Game"},
        {"quitGame", u8"Exit Game"},
        {"settings", u8"Settings"},
        {"volumeSFX", u8"SFX Volume"},
        {"volumeBGM", u8"BMG Volume"},
        {"bgmSelect", u8"BGM Select"},
        {"snakeColor", u8"Snake Color"},
        {"boardColor", u8"Board Color"},
        {"wallPassing", u8"Wall Passing"},
    }},
    { Language::JAPANESE, {
        {"gameTitle", u8"ヘビゲーム"},
        {"pressSpace", u8"スペースキーを押して開始"},
        {"pressQuit", u8"ESC/Qを押して終了"},
        {"easy", u8"簡単"},
        {"normal", u8"普通"},
        {"hard", u8"難しい"},
        {"difficultySelect", u8"難易度を選択"},
        {"gameOver", u8"ゲームオーバー"},
        {"score", u8"スコア"},
        {"english", u8"英語"},
        {"japanese", u8"日本語"},
        {"portuguese", u8"ポルトガル語"},
        {"pressEnter", u8"Enterキーを押して続行"},
        {"languageSelect", u8"言語の選択"},
        {"highscores", u8"ハイスコア"},
        {"highscoresTitle", u8"ハイスコア"},
        {"pressEnterToReturn", u8"Enterキーを押して戻る"},
        {"highscoreMsg", u8"ハイスコア達成！"},
        {"enterID", u8" IDを入力してください:"},
        {"yourID", u8"あなたの名前"},
        {"pressEnterToConfirm", u8"Enterキーを押して確認"},
        {"paused", u8"一時停止"},
        {"rank", u8"ランク"},
        {"id", u8"ID"},
        {"time", u8"日時"},
        {"startGame", u8"ゲームを始める"},
        {"quitGame", u8"ゲームを閉じる"},
        {"settings", u8"設定"},
        {"volumeSFX", u8"効果音の音量"},
        {"volumeBGM", u8"BGMの音量"},
        {"bgmSelect", u8"BGMの選択"},
        {"snakeColor", u8"ヘビの色"},
        {"boardColor", u8"ボードの色"},
        {"wallPassing", u8"壁を通り抜ける"},
    }},
};

std::string LanguageManager::getText(const std::string& key) {
    auto langMap = allStrings.find(currentLanguage);
    if (langMap != allStrings.end()) {
        auto stringIt = langMap->second.find(key);
        if (stringIt != langMap->second.end()) {
            return stringIt->second;
        }
    }
    return "KEY_NOT_FOUND";
}

void LanguageManager::setLanguage(Language lang) {
    currentLanguage = lang;
}