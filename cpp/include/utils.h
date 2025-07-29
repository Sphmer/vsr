#pragma once

#include <string>
#include <vector>
#include <map>
#include <any>
#include <filesystem>
#include <chrono>

namespace utils {

// String utilities
std::string trim(const std::string& str);
std::string toLower(const std::string& str);
std::string toUpper(const std::string& str);
std::vector<std::string> split(const std::string& str, const std::string& delimiter);
std::string join(const std::vector<std::string>& strings, const std::string& delimiter);
bool startsWith(const std::string& str, const std::string& prefix);
bool endsWith(const std::string& str, const std::string& suffix);
std::string replaceAll(const std::string& str, const std::string& from, const std::string& to);

// Numeric utilities
bool isNumeric(const std::string& str);
double toDouble(const std::string& str);
int toInt(const std::string& str);
std::string formatNumber(double value, int precision = 2);
std::string formatInteger(int value);

// File utilities
bool fileExists(const std::string& filepath);
bool directoryExists(const std::string& dirpath);
bool createDirectory(const std::string& dirpath);
std::string getFileExtension(const std::string& filepath);
std::string getFileName(const std::string& filepath);
std::string getDirectoryName(const std::string& filepath);
std::vector<std::string> listFiles(const std::string& directory, const std::string& extension = "");
std::string readFile(const std::string& filepath);
bool writeFile(const std::string& filepath, const std::string& content);

// Hash utilities
std::string calculateMD5(const std::string& input);
std::string calculateFileHash(const std::string& filepath);

// Time utilities
std::string getCurrentTimestamp();
std::string formatTimestamp(const std::chrono::system_clock::time_point& time);

// Console utilities
void enableUTF8Console();
std::pair<int, int> getConsoleSize();
void setConsoleTitle(const std::string& title);
void clearScreen();

// Data conversion utilities
std::string anyToString(const std::any& value);
std::any stringToAny(const std::string& str);
bool isValidJSON(const std::string& json_str);

// Platform detection
bool isWindows();
bool isMacOS();
bool isLinux();
std::string getPlatformName();

// Error handling
class VSRException : public std::exception {
public:
    explicit VSRException(const std::string& message) : message_(message) {}
    const char* what() const noexcept override { return message_.c_str(); }

private:
    std::string message_;
};

// Logging utilities
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR_LEVEL
};

void log(LogLevel level, const std::string& message);
void setLogLevel(LogLevel level);

} // namespace utils
