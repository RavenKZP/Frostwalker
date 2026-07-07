#pragma once

#include <unordered_set>

#include "REX/REX.h"

class Settings : public REX::Singleton<Settings> {
public:

    bool Init();  // Initialize settings, return true if successful

    void LoadMainIni();
    void SaveMainIni();

    void LoadSettings();   // Load Settings from file
    void SaveSettings();   // Save settings to file
    void ResetSettings();  // Restore default and save to file

    // Kill Switch
    bool ModActive = true;

    float minScale = 0.5f;
    float maxScale = 1.3f;

    float minLifetime = 10.0f;
    float maxLifetime = 60.0f;
    float meltingTime = 2.0f;

    float IceDistance = 100.0f;

    float damageScale = 50.0f;
    float updateInterval = 0.05f;

    std::vector<std::string> neutralSources;
    std::vector<std::string> coldSources;
    std::vector<std::string> fireSources;

    std::vector<RE::BGSHazard*> IceChunks;
    std::vector<RE::BGSHazard*> IceFloesL;
    std::vector<RE::BGSHazard*> IceFloesM;
    std::vector<RE::BGSHazard*> IceFloesS;

    std::vector<RE::BGSKeyword*> Frostwalker_FrostWalk_Keywords;
    std::vector<RE::BGSKeyword*> Frostwalker_FireWalk_Keywords;
};