#include "gui_process_manager.h"

namespace afglobal {

GUIProcessManager::GUIProcessManager() {}

GUIProcessManager::~GUIProcessManager() {}

void GUIProcessManager::RefreshProcessList() {
    // Process list is populated by the PI injector
    // This is a simple manager for the UI
}

const std::vector<std::pair<unsigned long, std::string>>& GUIProcessManager::GetProcesses() const {
    return processes_;
}

bool GUIProcessManager::RemoveProcess(unsigned long pid) {
    auto it = std::find_if(processes_.begin(), processes_.end(),
                          [pid](const auto& p) { return p.first == pid; });
    
    if (it != processes_.end()) {
        processes_.erase(it);
        return true;
    }
    return false;
}

size_t GUIProcessManager::GetProcessCount() const {
    return processes_.size();
}

}  // namespace afglobal
