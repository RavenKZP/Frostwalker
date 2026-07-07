#include "MCP.h"

#include "SKSEMCP/SKSEMenuFramework.hpp"
#include "Manager.h"
#include "Settings.h"

namespace MCP {

    void Register() {
        if (!SKSEMenuFramework::IsInstalled()) {
            logger::warn("SKSE Menu Framework is not installed. Cannot register menu.");
            return;
        }
        SKSEMenuFramework::SetSection("Frostwalker");
        SKSEMenuFramework::AddSectionItem("Settings", RenderSettings);
#ifndef NDEBUG
        SKSEMenuFramework::AddSectionItem("Log", RenderLog);
#endif

        logger::info("SKSE Menu Framework registered.");
    }

    void __stdcall RenderSettings() {
        auto* cfg = Settings::GetSingleton();
        ImGuiMCP::Checkbox("Mod Active", &cfg->ModActive);
        ImGuiMCP::InputFloat("Min Ice Floe Scale", &cfg->minScale);
        ImGuiMCP::DragFloat("##Min Ice Floe Scale", &cfg->minScale, 0.01f, 0.1f, 10.0f, "%.2f");
        ImGuiMCP::InputFloat("Max Ice Floe Scale", &cfg->maxScale);
        ImGuiMCP::DragFloat("##Max Ice Floe Scale", &cfg->maxScale, 0.01f, 0.1f, 10.0f, "%.2f");

        ImGuiMCP::InputFloat("Min Distance between Ice Floes", &cfg->IceDistance);
        ImGuiMCP::DragFloat("##Min Distance between Ice Floes", &cfg->IceDistance, 1.0f, 0.0f, 1000.0f, "%.2f");

        ImGuiMCP::InputFloat("Min Ice Floe Lifetime", &cfg->minLifetime);
        ImGuiMCP::DragFloat("##Min Ice Floe Lifetime", &cfg->minLifetime, 0.1f, 0.0f, 1000.0f, "%.2f");
        ImGuiMCP::InputFloat("Max Ice Floe Lifetime", &cfg->maxLifetime);
        ImGuiMCP::DragFloat("##Max Ice Floe Lifetime", &cfg->maxLifetime, 0.1f, 0.0f, 1000.0f, "%.2f");
        
        ImGuiMCP::InputFloat("Melting Time", &cfg->meltingTime);
        ImGuiMCP::DragFloat("##Melting Time", &cfg->meltingTime, 0.1f, 0.0f, 1000.0f, "%.2f");

        ImGuiMCP::InputFloat("Damage Scale", &cfg->damageScale);
        ImGuiMCP::DragFloat("##Damage Scale", &cfg->damageScale, 0.1f, 0.0f, 1000.0f, "%.2f");

        ImGuiMCP::InputFloat("Projectile Update Interval", &cfg->updateInterval);
        ImGuiMCP::DragFloat("##Projectile Update Interval", &cfg->updateInterval, 0.01f, 0.0f, 10.0f, "%.2f");


        if (ImGuiMCP::Button("Save Settings")) {
            cfg->SaveSettings();
        }
        if (ImGuiMCP::Button("Reset Settings")) {
            cfg->ResetSettings();
        }
    }

    void __stdcall MCP::RenderLog() {
        ImGuiMCP::Checkbox("Trace", &MCPLog::log_trace);
        ImGuiMCP::SameLine();
        ImGuiMCP::Checkbox("Info", &MCPLog::log_info);
        ImGuiMCP::SameLine();
        ImGuiMCP::Checkbox("Warning", &MCPLog::log_warning);
        ImGuiMCP::SameLine();
        ImGuiMCP::Checkbox("Error", &MCPLog::log_error);
        ImGuiMCP::InputText("Custom Filter", MCPLog::custom, 255);

        // if"Generate Log" button is pressed, read the log file
        if (ImGuiMCP::Button("Generate Log")) {
            logLines = MCPLog::ReadLogFile();
        }

        // Display each line in a new ImGuiMCP::Text() element
        for (const auto& line : logLines) {
            if (line.find("trace") != std::string::npos && !MCPLog::log_trace) continue;
            if (line.find("info") != std::string::npos && !MCPLog::log_info) continue;
            if (line.find("warning") != std::string::npos && !MCPLog::log_warning) continue;
            if (line.find("error") != std::string::npos && !MCPLog::log_error) continue;
            if (line.find(MCPLog::custom) == std::string::npos && MCPLog::custom != "") continue;
            ImGuiMCP::Text(line.c_str());
        }
    }
}

namespace MCPLog {
    std::filesystem::path GetLogPath() {
        const auto logsFolder = SKSE::log::log_directory();
        if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
        auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
        auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
        return logFilePath;
    }

    std::vector<std::string> ReadLogFile() {
        std::vector<std::string> logLines;

        // Open the log file
        std::ifstream file(GetLogPath().c_str());
        if (!file.is_open()) {
            // Handle error
            return logLines;
        }

        // Read and store each line from the file
        std::string line;
        while (std::getline(file, line)) {
            logLines.push_back(line);
        }

        file.close();

        return logLines;
    }
}