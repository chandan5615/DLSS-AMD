#pragma once

#include <vector>
#include <string>
#include <utility>

namespace afglobal {

class GUIProcessManager {
public:
    GUIProcessManager();
    ~GUIProcessManager();

    void RefreshProcessList();
    
    const std::vector<std::pair<unsigned long, std::string>>& GetProcesses() const;
    
    bool RemoveProcess(unsigned long pid);
    
    size_t GetProcessCount() const;

private:
    std::vector<std::pair<unsigned long, std::string>> processes_;
};

}  // namespace afglobal
