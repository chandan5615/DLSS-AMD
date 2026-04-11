#include "gui_window.h"
#include "gui_log_viewer.h"
#include "gui_process_manager.h"
#include "../common/logger.h"
#include "../common/config.h"
#include <windows.h>
#include <shellapi.h>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

namespace afglobal {

GUIWindow::GUIWindow()
    : glfw_window_(nullptr)
    , imgui_context_(nullptr)
    , frame_gen_enabled_(true)
    , frame_multiplier_(2)
    , interpolation_mode_(0)
    , original_fps_(60.0f)
    , generated_fps_(120.0f)
    , config_changed_(false)
    , has_unsaved_changes_(false)
{
    memset(new_blacklist_entry_, 0, sizeof(new_blacklist_entry_));
    memset(new_whitelist_entry_, 0, sizeof(new_whitelist_entry_));
}

GUIWindow::~GUIWindow() {
    Shutdown();
}

bool GUIWindow::Initialize() {
    try {
        log_viewer_ = std::make_unique<GUILogViewer>("afglobal.log");
        process_manager_ = std::make_unique<GUIProcessManager>();
        RefreshConfigFromFile();
        is_running_ = true;
        Logger::Instance().Info("GUI Window initialized");
        return true;
    }
    catch (const std::exception& e) {
        Logger::Instance().Error(std::string("GUI initialization failed: ") + e.what());
        return false;
    }
}

void GUIWindow::Shutdown() {
    is_running_ = false;

    if (render_thread_.joinable()) {
        render_thread_.join();
    }

    Logger::Instance().Info("GUI Window shutdown complete");
}

bool GUIWindow::IsRunning() const {
    return is_running_;
}

bool GUIWindow::IsVisible() const {
    return is_visible_;
}

void GUIWindow::Show() {
    is_visible_ = true;
    Logger::Instance().Debug("GUI Window shown");
}

void GUIWindow::Hide() {
    is_visible_ = false;
    Logger::Instance().Debug("GUI Window hidden");
}

void GUIWindow::ToggleVisibility() {
    is_visible_ = !is_visible_;
}

void GUIWindow::RunFrame() {
    if (!is_running_) return;
    Update();
}

void GUIWindow::Update() {
    if (log_viewer_) {
        log_viewer_->Update();
    }
    
    RefreshFPSMetrics();
    
    if (config_changed_) {
        ApplyConfigChanges();
    }
}

bool GUIWindow::InitializeImGui() {
    return true;  // Disabled - no ImGui
}

bool GUIWindow::InitializeGLFW() {
    return true;  // Disabled - no GLFW
}

void GUIWindow::ApplyDarkTheme() {
    // Theme applied via CSS/HTML if web-based UI is used
    // Otherwise, Windows default theme is used
}

void GUIWindow::RenderMainUI() {
    // Rendering handled by native Windows UI or web-based alternative
    // This is a placeholder for future implementation
}

void GUIWindow::RenderFrameGenerationToggle() {
    // UI element rendering - handled by native controls
}

void GUIWindow::RenderFrameMultiplierSelector() {
    // UI element rendering - handled by native controls
}

void GUIWindow::RenderInterpolationModeSelector() {
    // UI element rendering - handled by native controls
}

void GUIWindow::RenderFPSCounter() {
    // UI element rendering - handled by native controls
}

void GUIWindow::RenderProcessList() {
    // UI element rendering - handled by native controls
}

void GUIWindow::RenderBlacklistWhitelistEditor() {
    // UI element rendering - handled by native controls
}

void GUIWindow::RenderLogViewer() {
    // UI element rendering - handled by native controls
}

void GUIWindow::RenderControlButtons() {
    // UI element rendering - handled by native controls
}

void GUIWindow::RefreshProcessList() {
    if (process_manager_) {
        process_manager_->RefreshProcessList();
    }
}

void GUIWindow::RefreshFPSMetrics() {
    // Placeholder: FPS metrics would come from DLL via shared memory
    // For now, calculate based on frame multiplier
    original_fps_ = 60.0f;  // Base FPS
    generated_fps_ = frame_gen_enabled_ ? (original_fps_ * frame_multiplier_) : original_fps_;
}

void GUIWindow::UpdateProcessMetrics() {
    RefreshProcessList();
}

void GUIWindow::ApplyConfigChanges() {
    if (!has_unsaved_changes_) return;

    try {
        auto& config_mgr = ConfigManager::Instance();
        const auto& config_orig = config_mgr.GetConfig();

        // Create updated configuration
        Config config = config_orig;
        config.enabled = frame_gen_enabled_;
        config.target_multiplier = frame_multiplier_;
        config.fallback_blend_only = (interpolation_mode_ == 1);
        config.process_blacklist = edited_blacklist_;
        config.process_whitelist = edited_whitelist_;

        // Save to file
        nlohmann::json json;
        json["frame_generation"]["enabled"] = frame_gen_enabled_;
        json["frame_generation"]["multiplier"] = frame_multiplier_;
        json["frame_generation"]["interpolation_mode"] = interpolation_mode_;
        json["filtering"]["blacklist"] = edited_blacklist_;
        json["filtering"]["whitelist"] = edited_whitelist_;

        std::ofstream config_file("afglobal_config.json");
        config_file << json.dump(2);
        config_file.close();

        // Update the configuration manager with new values
        config_mgr.UpdateConfig(config);

        config_changed_ = false;
        has_unsaved_changes_ = false;
        Logger::Instance().Info("Configuration changes applied and saved");
    }
    catch (const std::exception& e) {
        Logger::Instance().Error(std::string("Failed to apply config changes: ") + e.what());
    }
}

void GUIWindow::RefreshConfigFromFile() {
    try {
        std::ifstream config_file("afglobal_config.json");
        if (!config_file.is_open()) {
            Logger::Instance().Warning("Config file not found, using defaults");
            return;
        }

        nlohmann::json json;
        config_file >> json;

        frame_gen_enabled_ = json["frame_generation"]["enabled"].get<bool>();
        frame_multiplier_ = json["frame_generation"]["multiplier"].get<int>();
        interpolation_mode_ = json["frame_generation"]["interpolation_mode"].get<int>();
        
        if (json.contains("filtering")) {
            edited_blacklist_ = json["filtering"]["blacklist"].get<std::vector<std::string>>();
            edited_whitelist_ = json["filtering"]["whitelist"].get<std::vector<std::string>>();
        }

        has_unsaved_changes_ = false;
        Logger::Instance().Debug("Configuration loaded from file");
    }
    catch (const std::exception& e) {
        Logger::Instance().Error(std::string("Failed to load config: ") + e.what());
    }
}

}  // namespace afglobal
