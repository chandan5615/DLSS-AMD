#include "gui_log_viewer.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace afglobal {

GUILogViewer::GUILogViewer(const std::string& logFilePath)
    : log_file_path_(logFilePath), max_lines_(50), last_read_pos_(0), auto_scroll_(true) {}

GUILogViewer::~GUILogViewer() {}

void GUILogViewer::Update() {
    ReadLogFile();
}

void GUILogViewer::Render() {
    // Rendering is handled by the main GUI window
    // This just manages the log data
}

void GUILogViewer::SetMaxLines(size_t maxLines) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    max_lines_ = maxLines;
}

const std::deque<std::string>& GUILogViewer::GetLines() const {
    return log_lines_;
}

void GUILogViewer::ReadLogFile() {
    std::ifstream file(log_file_path_, std::ios::binary);
    if (!file.is_open()) {
        return;
    }

    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();

    if (file_size == last_read_pos_ && file_size > 0) {
        // No new content
        return;
    }

    file.seekg(0, std::ios::beg);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    std::istringstream stream(content);
    std::string line;

    {
        std::lock_guard<std::mutex> lock(log_mutex_);
        log_lines_.clear();

        while (std::getline(stream, line)) {
            if (!line.empty()) {
                log_lines_.push_back(line);
            }
        }

        // Keep only last max_lines_
        while (log_lines_.size() > max_lines_) {
            log_lines_.pop_front();
        }
    }

    last_read_pos_ = file_size;
}

void GUILogViewer::ParseLogLine(const std::string& line) {
    // Log lines are in format: [YYYY-MM-DD HH:MM:SS] [LEVEL] Message
    // For now, just store as-is
}

}  // namespace afglobal
