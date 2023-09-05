#pragma once

namespace config {
    extern const char* INI_FILE_NAME;
    extern int iMovementType;
    extern bool bEnableNotifications;

    std::string GetIniFilePath();
    int GetIniSetting(const char* settingName, int defaultValue);
    void SetIniSetting(const char* settingName, int value);
    std::unordered_map<std::string, int> ReadSettings();
}