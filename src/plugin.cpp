#include "logger.h"

#include "Manager.h"
#include "MCP.h"
#include "Hooks.h"
#include "Settings.h"

void OnMessage(SKSE::MessagingInterface::Message* message) {
    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        if (Settings::GetSingleton()->Init()) {  // Init
            Hooks::InstallHooks();
            MCP::Register();
        } else {
            logger::error("Failed to initialize settings. Disabling mod.");
            RE::DebugMessageBox("Frostwalker: Failed to initialize settings. Disabling mod.");
        }
    } else if (message->type == SKSE::MessagingInterface::kPostLoadGame) {
        if (Settings::GetSingleton()->ModActive) {
            Frostwalker::Manager::GetSingleton()->OnGameLoad();
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    SetupLog();
    logger::info("Plugin loaded");
    SKSE::Init(skse);
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
    return true;
}
