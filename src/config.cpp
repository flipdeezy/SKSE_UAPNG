#include "config.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <Windows.h>

namespace config {
    const char* INI_FILE_NAME = "UAPNG.ini";
    int iMovementType;
    bool bEnableNotifications;
    bool bEnableSpamPotions;
    std::unordered_set<std::string> allowedPotionNames;

    std::string GetIniFilePath() {
        char dllPath[MAX_PATH];
        GetModuleFileNameA(GetModuleHandleA("UAPNG.dll"), dllPath, sizeof(dllPath));
        std::string iniFilePath(dllPath);
        size_t lastSlashIndex = iniFilePath.find_last_of("\\/");
        iniFilePath = iniFilePath.substr(0, lastSlashIndex + 1) + INI_FILE_NAME;
        return iniFilePath;
    }

    int GetIniSetting(const char* settingName, int defaultValue) {
        std::string iniFilePath = GetIniFilePath();
        return GetPrivateProfileIntA("Settings", settingName, defaultValue, iniFilePath.c_str());
    }

    void SetIniSetting(const char* settingName, int value) {
        std::string iniFilePath = GetIniFilePath();
        WritePrivateProfileStringA("Settings", settingName, std::to_string(value).c_str(), iniFilePath.c_str());
    }

    void ReadAllowedPotionNames() {
        std::string iniFilePath = GetIniFilePath();
        char buffer[4096];
        GetPrivateProfileSectionA("AllowedPotions", buffer, sizeof(buffer), iniFilePath.c_str());

        allowedPotionNames.clear();
        char* ptr = buffer;
        while (*ptr != '\0') {
            std::string potionName(ptr);
            allowedPotionNames.insert(potionName);
            ptr += potionName.size() + 1;
        }
    }

    std::unordered_map<std::string, int> ReadSettings() {
        std::unordered_map<std::string, int> settings;
        settings["iMovementType"] = GetIniSetting("iMovementType", 0);
        settings["bEnableNotifications"] = GetIniSetting("bEnableNotifications", 1);
        settings["bEnableSpamPotions"] = GetIniSetting("bEnableSpamPotions", 1);
        ReadAllowedPotionNames();
        return settings;
    }
}
