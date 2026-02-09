#pragma once

#include <string>
#include <map>
#include "Main.h"

// The LanguageManager class is responsible for handling all language-related strings.
class LanguageManager {
public:
    static std::string getText(const std::string& key);
    static void setLanguage(Language lang);
};