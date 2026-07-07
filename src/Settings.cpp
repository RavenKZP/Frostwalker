#include "Settings.h"

#include "Utils.h"

bool Settings::Init() {
    auto Frostwalker_IceFloeChunkS01 = RE::TESForm::LookupByEditorID<RE::BGSHazard>("Frostwalker_IceFloeChunkS01");
    auto Frostwalker_IceFloeChunkS02 = RE::TESForm::LookupByEditorID<RE::BGSHazard>("Frostwalker_IceFloeChunkS02");
    auto Frostwalker_IceFloeChunkS03 = RE::TESForm::LookupByEditorID<RE::BGSHazard>("Frostwalker_IceFloeChunkS03");

    auto Frostwalker_IceFloeL01 = RE::TESForm::LookupByEditorID<RE::BGSHazard>("Frostwalker_IceFloeL01");
    auto Frostwalker_IceFloeM01 = RE::TESForm::LookupByEditorID<RE::BGSHazard>("Frostwalker_IceFloeM01");
    auto Frostwalker_IceFloeS01 = RE::TESForm::LookupByEditorID<RE::BGSHazard>("Frostwalker_IceFloeS01");
    auto Frostwalker_IceFloeS02 = RE::TESForm::LookupByEditorID<RE::BGSHazard>("Frostwalker_IceFloeS02");

    auto Frostwalker_FrostWalk_Keyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("Frostwalker_FrostWalk_Keyword");
    auto Frostwalker_FrostCloak_Keyword =
        RE::TESForm::LookupByEditorID<RE::BGSKeyword>("Frostwalker_FrostCloak_Keyword");


    auto Frostwalker_FireWalk_Keyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("Frostwalker_FireWalk_Keyword");
    auto Frostwalker_FireCloak_Keyword =
        RE::TESForm::LookupByEditorID<RE::BGSKeyword>("Frostwalker_FireCloak_Keyword");


    if (!Frostwalker_IceFloeChunkS01 || !Frostwalker_IceFloeChunkS02 || !Frostwalker_IceFloeChunkS03 ||
        !Frostwalker_IceFloeL01 || !Frostwalker_IceFloeM01 || !Frostwalker_IceFloeS01 || !Frostwalker_IceFloeS02 ||
        !Frostwalker_FrostWalk_Keyword || !Frostwalker_FrostCloak_Keyword || !Frostwalker_FireWalk_Keyword ||
        !Frostwalker_FireCloak_Keyword) {
        logger::error("Failed to find one or more required forms. Please ensure Frostwalker.esp is loaded.");
        ModActive = false;
        return false;
    }
    IceChunks.push_back(Frostwalker_IceFloeChunkS01);
    IceChunks.push_back(Frostwalker_IceFloeChunkS02);
    IceChunks.push_back(Frostwalker_IceFloeChunkS03);

    IceFloesL.push_back(Frostwalker_IceFloeL01);
    IceFloesM.push_back(Frostwalker_IceFloeM01);
    IceFloesS.push_back(Frostwalker_IceFloeS01);
    IceFloesS.push_back(Frostwalker_IceFloeS02);

    Frostwalker_FrostWalk_Keywords.push_back(Frostwalker_FrostWalk_Keyword);
    Frostwalker_FrostWalk_Keywords.push_back(Frostwalker_FrostCloak_Keyword);

    Frostwalker_FireWalk_Keywords.push_back(Frostwalker_FireWalk_Keyword);
    Frostwalker_FireWalk_Keywords.push_back(Frostwalker_FireCloak_Keyword);

    LoadSettings();
	return true;
}

static RE::BGSKeyword* GetOrCreateKeyword(std::string kwd) {
    auto form = RE::TESForm::LookupByEditorID(kwd);
    if (form) {
        if (auto keyword = form->As<RE::BGSKeyword>()) {
            return keyword;
        } else {
            logger::error("Form {} found but is not a keyword", kwd);
            return nullptr;
        }
    }
    logger::warn("Keyword {} not found, creating it", kwd);
    const auto factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSKeyword>();
    RE::BGSKeyword* createdkeyword = nullptr;
    if (createdkeyword = factory ? factory->Create() : nullptr; createdkeyword) {
        createdkeyword->formEditorID = kwd;
    }
    return createdkeyword;
}

static void LoadKeywordsFromFile(const std::filesystem::path& path, std::vector<RE::BGSKeyword*>& outKeywords) {
    std::ifstream in(path);
    if (!in.is_open()) {
        logger::warn("Failed to open keyword config '{}'", path.string());
        return;
    }
    logger::info("Loading keywords from '{}'", path.string());
    std::string line;
    while (std::getline(in, line)) {
        // trim whitespace
        auto first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) continue;
        auto last = line.find_last_not_of(" \t\r\n");
        std::string trimmed = line.substr(first, last - first + 1);
        if (trimmed.empty() || trimmed[0] == '#') continue;
        auto keyword = GetOrCreateKeyword(trimmed);
        if (keyword) {
            outKeywords.push_back(keyword);
        } else {
            logger::error("Keyword '{}' not found in game data", trimmed);
        }  
    }
}

static void LoadPatternsFromFile(const std::filesystem::path& path, std::vector<std::string>& outPatterns) {
    std::ifstream in(path);
    if (!in.is_open()) {
        logger::warn("Failed to open pattern config '{}'", path.string());
        return;
    }
    logger::info("Loading patterns from '{}'", path.string());
    std::string line;
    while (std::getline(in, line)) {
        // trim whitespace
        auto first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) continue;
        auto last = line.find_last_not_of(" \t\r\n");
        std::string trimmed = line.substr(first, last - first + 1);
        if (trimmed.empty() || trimmed[0] == '#') continue;
        outPatterns.push_back(Utils::ToLower(trimmed));  // store lowercase
    }
}

static void LoadAllPatterns(const std::filesystem::path& folder, std::vector<std::string>& outPatterns) {
    if (!std::filesystem::exists(folder)) return;
    for (auto& entry : std::filesystem::directory_iterator(folder)) {
        if (!entry.is_regular_file()) continue;
        LoadPatternsFromFile(entry.path(), outPatterns);
    }
}

static void LoadAllKeywords(const std::filesystem::path& folder, std::vector<RE::BGSKeyword*>& outKeywords) {
    if (!std::filesystem::exists(folder)) return;
    for (auto& entry : std::filesystem::directory_iterator(folder)) {
        if (!entry.is_regular_file()) continue;
        LoadKeywordsFromFile(entry.path(), outKeywords);
    }
}

void Settings::LoadMainIni() {
    std::ifstream file("Data\\SKSE\\Plugins\\Frostwalker.ini");
    logger::info("Loading settings from INI");
    if (!file.is_open()) {
        logger::error("INI not found");
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.contains("bModActive")) {
            auto pos = line.find('=');
            if (pos != std::string::npos) {
                std::string value = line.substr(pos + 1);
                if (value == "True" || value == "true" || value == "1") {
                    ModActive = true;
                } else{
                    ModActive = false;
                }
            }
        }
        if (line.contains("fMinScale")) {
            auto pos = line.find('=');
            if (pos != std::string::npos) {
                std::string value = line.substr(pos + 1);
                minScale = std::stof(value);
            }
        }
        if (line.contains("fMaxScale")) {
            auto pos = line.find('=');
            if (pos != std::string::npos) {
                std::string value = line.substr(pos + 1);
                maxScale = std::stof(value);
            }
        }
        if (line.contains("fMinLifetime")) {
            auto pos = line.find('=');
            if (pos != std::string::npos) {
                std::string value = line.substr(pos + 1);
                minLifetime = std::stof(value);
            }
        }
        if (line.contains("fMaxLifetime")) {
            auto pos = line.find('=');
            if (pos != std::string::npos) {
                std::string value = line.substr(pos + 1);
                maxLifetime = std::stof(value);
            }
        }
        if (line.contains("fMeltingTime")) {
            auto pos = line.find('=');
            if (pos != std::string::npos) {
                std::string value = line.substr(pos + 1);
                meltingTime = std::stof(value);
            }
        }
        if (line.contains("fIceDistance")) {
            auto pos = line.find('=');
            if (pos != std::string::npos) {
                std::string value = line.substr(pos + 1);
                IceDistance = std::stof(value);
            }
        }
        if (line.contains("fDamageScale")) {
            auto pos = line.find('=');
            if (pos != std::string::npos) {
                std::string value = line.substr(pos + 1);
                damageScale = std::stof(value);
            }
        }
        if (line.contains("fUpdateInterval")) {
            auto pos = line.find('=');
            if (pos != std::string::npos) {
                std::string value = line.substr(pos + 1);
                updateInterval = std::stof(value);
            }
        }
    }
    logger::info("Settings loaded from INI");
}

void Settings::SaveMainIni() {
    std::ofstream file("Data\\SKSE\\Plugins\\Frostwalker.ini");
    if (!file.is_open()) {
        logger::error("Failed to open INI for writing");
        return;
    }
    file << "bModActive=" << (ModActive ? "True" : "False") << "\n";
    file << "fMinScale=" << minScale << "\n";
    file << "fMaxScale=" << maxScale << "\n";
    file << "fMinLifetime=" << minLifetime << "\n";
    file << "fMaxLifetime=" << maxLifetime << "\n";
    file << "fMeltingTime=" << meltingTime << "\n";
    file << "fIceDistance=" << IceDistance << "\n";
    file << "fDamageScale=" << damageScale << "\n";
    file << "fUpdateInterval=" << updateInterval << "\n";
    logger::info("Settings saved to INI");
}

void Settings::LoadSettings() {
    LoadMainIni();

    LoadAllPatterns("Data\\SKSE\\Plugins\\Frostwalker\\NeutralSources", neutralSources);
    LoadAllPatterns("Data\\SKSE\\Plugins\\Frostwalker\\FrostSources", coldSources);
    LoadAllPatterns("Data\\SKSE\\Plugins\\Frostwalker\\FireSources", fireSources);

    LoadAllKeywords("Data\\SKSE\\Plugins\\Frostwalker\\FrostKeywords", Frostwalker_FrostWalk_Keywords);
    LoadAllKeywords("Data\\SKSE\\Plugins\\Frostwalker\\FireKeywords", Frostwalker_FireWalk_Keywords);

    logger::info("Loaded {} neutral source patterns", neutralSources.size());
    logger::info("Loaded {} fire source patterns", fireSources.size());
    logger::info("Loaded {} frost source patterns", coldSources.size());

    logger::info("Loaded {} FrostWalk Keywords", Frostwalker_FrostWalk_Keywords.size());
    logger::info("Loaded {} FireWalk Keywords", Frostwalker_FireWalk_Keywords.size());
}


void Settings::SaveSettings() { SaveMainIni(); }

void Settings::ResetSettings() {
    ModActive = true;

    minScale = 0.5f;
    maxScale = 1.3f;

    minLifetime = 10.0f;
    maxLifetime = 60.0f;
    meltingTime = 2.0f;

    IceDistance = 100.0f;

    damageScale = 50.0f;
    updateInterval = 0.05f;

    SaveSettings();
}