#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace config {
    extern int iMovementType;
    extern bool bEnableNotifications;
    extern bool bEnableSpamPotions;
    extern std::unordered_set<std::string> allowedPotionNames;

    std::unordered_map<std::string, int> ReadSettings();
    void ReadAllowedPotionNames();
    std::string GetIniFilePath();
    int GetIniSetting(const char* settingName, int defaultValue);
    void SetIniSetting(const char* settingName, int value); 
}
