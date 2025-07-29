#include "utils.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <regex>
#include <filesystem>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>
#elif defined(__APPLE__)
#include <sys/ioctl.h>
#include <unistd.h>
#elif defined(__linux__)
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace utils {

// String utilities
std::string trim(const std::string& str) {
    const std::string whitespace = " \t\n\r\f\v";
    size_t first = str.find_first_not_of(whitespace);
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(whitespace);
    return str.substr(first, (last - first + 1));
}

std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);
    
    while (end != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    
    tokens.push_back(str.substr(start));
    return tokens;
}

std::string join(const std::vector<std::string>& strings, const std::string& delimiter) {
    if (strings.empty()) return "";
    
    std::ostringstream result;
    for (size_t i = 0; i < strings.size(); ++i) {
        if (i > 0) result << delimiter;
        result << strings[i];
    }
    return result.str();
}

bool startsWith(const std::string& str, const std::string& prefix) {
    return str.length() >= prefix.length() && 
           str.compare(0, prefix.length(), prefix) == 0;
}

bool endsWith(const std::string& str, const std::string& suffix) {
    return str.length() >= suffix.length() && 
           str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

std::string replaceAll(const std::string& str, const std::string& from, const std::string& to) {
    std::string result = str;
    size_t start_pos = 0;
    while ((start_pos = result.find(from, start_pos)) != std::string::npos) {
        result.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return result;
}

// Numeric utilities
bool isNumeric(const std::string& str) {
    if (str.empty()) return false;
    
    std::istringstream iss(str);
    double d;
    iss >> std::noskipws >> d;
    return iss.eof() && !iss.fail();
}

double toDouble(const std::string& str) {
    try {
        return std::stod(str);
    } catch (...) {
        return 0.0;
    }
}

int toInt(const std::string& str) {
    try {
        return std::stoi(str);
    } catch (...) {
        return 0;
    }
}

std::string formatNumber(double value, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

std::string formatInteger(int value) {
    return std::to_string(value);
}

// File utilities
bool fileExists(const std::string& filepath) {
    return std::filesystem::exists(filepath);
}

bool directoryExists(const std::string& dirpath) {
    return std::filesystem::exists(dirpath) && std::filesystem::is_directory(dirpath);
}

bool createDirectory(const std::string& dirpath) {
    try {
        return std::filesystem::create_directories(dirpath);
    } catch (...) {
        return false;
    }
}

std::string getFileExtension(const std::string& filepath) {
    std::filesystem::path path(filepath);
    return path.extension().string();
}

std::string getFileName(const std::string& filepath) {
    std::filesystem::path path(filepath);
    return path.filename().string();
}

std::string getDirectoryName(const std::string& filepath) {
    std::filesystem::path path(filepath);
    return path.parent_path().string();
}

std::vector<std::string> listFiles(const std::string& directory, const std::string& extension) {
    std::vector<std::string> files;
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (extension.empty() || endsWith(filename, extension)) {
                    files.push_back(filename);
                }
            }
        }
    } catch (...) {
        // Directory doesn't exist or can't be read
    }
    
    return files;
}

std::string readFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw VSRException("Cannot open file: " + filepath);
    }
    
    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}

bool writeFile(const std::string& filepath, const std::string& content) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        return false;
    }
    
    file << content;
    return file.good();
}

// Hash utilities (simplified MD5-like hash)
std::string calculateMD5(const std::string& input) {
    std::hash<std::string> hasher;
    size_t hash = hasher(input);
    
    std::ostringstream oss;
    oss << std::hex << hash;
    return oss.str();
}

std::string calculateFileHash(const std::string& filepath) {
    try {
        std::string content = readFile(filepath);
        return calculateMD5(filepath + ":" + content);
    } catch (...) {
        return calculateMD5(filepath);
    }
}

// Time utilities
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string formatTimestamp(const std::chrono::system_clock::time_point& time) {
    auto time_t = std::chrono::system_clock::to_time_t(time);
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// Console utilities
void enableUTF8Console() {
#ifdef _WIN32
    // Enable UTF-8 console output on Windows
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    // Enable virtual terminal processing for ANSI escape sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
    
    // Force console output to be unbuffered
    std::cout.setf(std::ios::unitbuf);
    std::cerr.setf(std::ios::unitbuf);
    
    // Note: _setmode with _O_U8TEXT causes garbled output with std::cout
    // Using SetConsoleOutputCP(CP_UTF8) above is sufficient for UTF-8 support
#endif
}

std::pair<int, int> getConsoleSize() {
    int width = 80, height = 24; // Default values
    
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }
#elif defined(__APPLE__) || defined(__linux__)
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        width = w.ws_col;
        height = w.ws_row;
    }
#endif
    
    return {width, height};
}

void setConsoleTitle(const std::string& title) {
#ifdef _WIN32
    SetConsoleTitleA(title.c_str());
#else
    std::cout << "\033]0;" << title << "\007" << std::flush;
#endif
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    // Alternative method using ANSI escape codes for better compatibility
    std::cout << "\033[2J\033[H" << std::flush;
}

// Data conversion utilities
std::string anyToString(const std::any& value) {
    try {
        if (value.type() == typeid(std::string)) {
            return std::any_cast<std::string>(value);
        } else if (value.type() == typeid(int)) {
            return std::to_string(std::any_cast<int>(value));
        } else if (value.type() == typeid(double)) {
            return formatNumber(std::any_cast<double>(value), 2);
        } else if (value.type() == typeid(bool)) {
            return std::any_cast<bool>(value) ? "true" : "false";
        }
    } catch (...) {
        // Fall through to default
    }
    
    return "N/A";
}

std::any stringToAny(const std::string& str) {
    // Try to convert to appropriate type
    if (str == "true" || str == "false") {
        return std::any(str == "true");
    } else if (isNumeric(str)) {
        if (str.find('.') != std::string::npos) {
            return std::any(toDouble(str));
        } else {
            return std::any(toInt(str));
        }
    }
    
    return std::any(str);
}

bool isValidJSON(const std::string& json_str) {
    // Simple JSON validation - check for basic structure
    std::string trimmed = trim(json_str);
    if (trimmed.empty()) {
        return false;
    }
    return (trimmed.front() == '{' && trimmed.back() == '}') ||
           (trimmed.front() == '[' && trimmed.back() == ']');
}

// Platform detection
bool isWindows() {
#ifdef _WIN32
    return true;
#else
    return false;
#endif
}

bool isMacOS() {
#ifdef __APPLE__
    return true;
#else
    return false;
#endif
}

bool isLinux() {
#ifdef __linux__
    return true;
#else
    return false;
#endif
}

std::string getPlatformName() {
    if (isWindows()) return "Windows";
    if (isMacOS()) return "macOS";
    if (isLinux()) return "Linux";
    return "Unknown";
}

// Logging utilities
static LogLevel current_log_level = LogLevel::INFO;

void log(LogLevel level, const std::string& message) {
    if (level < current_log_level) return;
    
    std::string level_str;
    switch (level) {
        case LogLevel::DEBUG: level_str = "DEBUG"; break;
        case LogLevel::INFO: level_str = "INFO"; break;
        case LogLevel::WARNING: level_str = "WARNING"; break;
        case LogLevel::ERROR_LEVEL: level_str = "ERROR"; break;
    }
    
    std::cout << "[" << level_str << "] " << message << std::endl;
}

void setLogLevel(LogLevel level) {
    current_log_level = level;
}

} // namespace utils
