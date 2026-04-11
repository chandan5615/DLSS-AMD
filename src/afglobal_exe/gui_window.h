#pragma once

#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <atomic>

namespace afglobal {

class GUILogViewer;
class GUIProcessManager;

class GUIWindow {
public:
    GUIWindow();
    ~GUIWindow();

    bool Initialize();
    void Shutdown();
    
    bool IsRunning() const;
    bool IsVisible() const;
    
    void Show();
    void Hide();
    void ToggleVisibility();
    
    // Main rendering loop
    void RunFrame();
    
    // Process updates
    void Update();

private:
    // Initialization helpers
    bool InitializeImGui();
    bool InitializeGLFW();
    void ApplyDarkTheme();

    // Rendering (placeholder methods for future UI implementation)
    void RenderMainUI();
    void RenderFrameGenerationToggle();
    void RenderFrameMultiplierSelector();
    void RenderInterpolationModeSelector();
    void RenderFPSCounter();
    void RenderProcessList();
    void RenderBlacklistWhitelistEditor();
    void RenderLogViewer();
    void RenderControlButtons();
    
    // Data refresh
    void RefreshProcessList();
    void RefreshFPSMetrics();
    void UpdateProcessMetrics();

    // Configuration sync
    void ApplyConfigChanges();
    void RefreshConfigFromFile();

    // Opaque pointers for potential future UI implementations
    void* glfw_window_;
    void* imgui_context_;
    
    std::unique_ptr<GUILogViewer> log_viewer_;
    std::unique_ptr<GUIProcessManager> process_manager_;

    std::atomic<bool> is_running_{false};
    std::atomic<bool> is_visible_{false};
    std::thread render_thread_;

    // UI State
    bool frame_gen_enabled_;
    int frame_multiplier_;
    int interpolation_mode_;  // 0 = Optical Flow, 1 = Blend Only
    
    // FPS Metrics
    float original_fps_;
    float generated_fps_;
    std::vector<float> fps_history_;

    // Processes
    std::vector<std::pair<unsigned long, std::string>> injected_processes_;
    
    // Edit buffers
    char new_blacklist_entry_[256];
    char new_whitelist_entry_[256];
    std::vector<std::string> edited_blacklist_;
    std::vector<std::string> edited_whitelist_;

    // Configuration state
    bool config_changed_;
    bool has_unsaved_changes_;
};

}  // namespace afglobal

