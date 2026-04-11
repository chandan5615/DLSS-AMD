#pragma once

#include <string>
#include <vector>
#include <deque>
#include <mutex>

namespace afglobal {

class GUILogViewer {
public:
    GUILogViewer(const std::string& logFilePath);
    ~GUILogViewer();

    void Update();
    void Render();
    
    void SetMaxLines(size_t maxLines);
    const std::deque<std::string>& GetLines() const;

private:
    void ReadLogFile();
    void ParseLogLine(const std::string& line);

    std::string log_file_path_;
    std::deque<std::string> log_lines_;
    size_t max_lines_;
    
    std::mutex log_mutex_;
    size_t last_read_pos_;
    bool auto_scroll_;
};

}  // namespace afglobal
