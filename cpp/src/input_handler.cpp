#include "input_handler.h"
#include "utils.h"
#include <iostream>
#include <algorithm>
#include <cctype>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#endif

InputHandler::InputHandler() {
    utils::log(utils::LogLevel::DEBUG, "InputHandler initialized");
}

std::string InputHandler::getKeyInput() {
    std::string input;
    
#ifdef _WIN32
    // Windows implementation using _getch()
    int ch = _getch();
    
    // Handle special keys
    if (ch == 0 || ch == 224) { // Extended key prefix
        ch = _getch();
        switch (ch) {
            case 72: return "up";
            case 80: return "down";
            case 75: return "left";
            case 77: return "right";
            case 73: return "pageup";
            case 81: return "pagedown";
            case 71: return "home";
            case 79: return "end";
        }
    } else {
        // Regular key
        switch (ch) {
            case 27: return "escape";
            case 13: return "enter";
            case 8: return "backspace";
            case 9: return "tab";
            case 'q': case 'Q': return "q";
            case 'h': case 'H': return "h";
            case 'r': case 'R': return "r";
            case 't': case 'T': return "t";
            case 'b': case 'B': return "b";
            case 'm': case 'M': return "m";
            case 'k': case 'K': return "up";
            case 'j': case 'J': return "down";
            case 'l': case 'L': return "right";
            default:
                if (std::isprint(ch)) {
                    return std::string(1, std::tolower(ch));
                }
        }
    }
#else
    // Unix/Linux implementation
    struct termios old_termios, new_termios;
    tcgetattr(STDIN_FILENO, &old_termios);
    new_termios = old_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    
    int ch = getchar();
    
    if (ch == 27) { // ESC sequence
        ch = getchar();
        if (ch == '[') {
            ch = getchar();
            switch (ch) {
                case 'A': input = "up"; break;
                case 'B': input = "down"; break;
                case 'C': input = "right"; break;
                case 'D': input = "left"; break;
                case '5': 
                    getchar(); // consume '~'
                    input = "pageup"; 
                    break;
                case '6': 
                    getchar(); // consume '~'
                    input = "pagedown"; 
                    break;
                case 'H': input = "home"; break;
                case 'F': input = "end"; break;
            }
        } else {
            input = "escape";
        }
    } else {
        switch (ch) {
            case 'q': case 'Q': input = "q"; break;
            case 'h': case 'H': input = "h"; break;
            case 'r': case 'R': input = "r"; break;
            case 't': case 'T': input = "t"; break;
            case 'b': case 'B': input = "b"; break;
            case 'm': case 'M': input = "m"; break;
            case 'k': case 'K': input = "up"; break;
            case 'j': case 'J': input = "down"; break;
            case 'l': case 'L': input = "right"; break;
            case 10: case 13: input = "enter"; break;
            case 127: case 8: input = "backspace"; break;
            case 9: input = "tab"; break;
            default:
                if (std::isprint(ch)) {
                    input = std::string(1, std::tolower(ch));
                }
        }
    }
    
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
#endif
    
    return normalizeInput(input);
}

std::string InputHandler::getLineInput() {
    std::string line;
    std::getline(std::cin, line);
    return utils::trim(line);
}

std::string InputHandler::normalizeInput(const std::string& input) {
    std::string normalized = utils::toLower(utils::trim(input));
    
    // Map alternative keys to standard names
    if (normalized == "quit" || normalized == "exit") {
        return "q";
    }
    
    if (normalized == "help") {
        return "h";
    }
    
    if (normalized == "reconfigure" || normalized == "config") {
        return "r";
    }
    
    if (normalized == "table") {
        return "t";
    }
    
    if (normalized == "bars" || normalized == "bar") {
        return "b";
    }
    
    if (normalized == "mixed" || normalized == "mix") {
        return "m";
    }
    
    return normalized;
}

bool InputHandler::validateInput(const std::string& input, const std::vector<std::string>& valid_inputs) {
    return std::find(valid_inputs.begin(), valid_inputs.end(), input) != valid_inputs.end();
}

std::string InputHandler::mapInput(const std::string& input, const std::map<std::string, std::string>& input_map) {
    auto it = input_map.find(input);
    if (it != input_map.end()) {
        return it->second;
    }
    return input;
}

int InputHandler::getMenuSelection(const std::vector<std::string>& options) {
    if (options.empty()) {
        return -1;
    }
    
    std::cout << "Select an option:" << std::endl;
    for (size_t i = 0; i < options.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << options[i] << std::endl;
    }
    
    std::cout << "Enter choice (1-" << options.size() << "): ";
    
    std::string input = getLineInput();
    int choice = utils::toInt(input);
    
    if (choice >= 1 && choice <= static_cast<int>(options.size())) {
        return choice - 1; // Return 0-based index
    }
    
    return -1; // Invalid selection
}

bool InputHandler::confirmAction(const std::string& message) {
    std::cout << message << " (y/n): ";
    
    std::string input = getLineInput();
    std::string normalized = utils::toLower(input);
    
    return (normalized == "y" || normalized == "yes");
}

std::string InputHandler::getStringInput(const std::string& prompt, const std::string& default_value) {
    std::cout << prompt;
    if (!default_value.empty()) {
        std::cout << " (default: " << default_value << ")";
    }
    std::cout << ": ";
    
    std::string input = getLineInput();
    
    if (input.empty() && !default_value.empty()) {
        return default_value;
    }
    
    return input;
}

int InputHandler::getIntInput(const std::string& prompt, int default_value, int min_value, int max_value) {
    std::cout << prompt;
    if (default_value != INT_MIN) {
        std::cout << " (default: " << default_value << ")";
    }
    if (min_value != INT_MIN || max_value != INT_MAX) {
        std::cout << " [" << min_value << "-" << max_value << "]";
    }
    std::cout << ": ";
    
    std::string input = getLineInput();
    
    if (input.empty() && default_value != INT_MIN) {
        return default_value;
    }
    
    int value = utils::toInt(input);
    
    // Validate range
    if (min_value != INT_MIN && value < min_value) {
        value = min_value;
    }
    if (max_value != INT_MAX && value > max_value) {
        value = max_value;
    }
    
    return value;
}

double InputHandler::getDoubleInput(const std::string& prompt, double default_value, double min_value, double max_value) {
    std::cout << prompt;
    if (default_value != -DBL_MAX) {
        std::cout << " (default: " << utils::formatNumber(default_value, 2) << ")";
    }
    if (min_value != -DBL_MAX || max_value != DBL_MAX) {
        std::cout << " [" << utils::formatNumber(min_value, 2) << "-" << utils::formatNumber(max_value, 2) << "]";
    }
    std::cout << ": ";
    
    std::string input = getLineInput();
    
    if (input.empty() && default_value != -DBL_MAX) {
        return default_value;
    }
    
    double value = utils::toDouble(input);
    
    // Validate range
    if (min_value != -DBL_MAX && value < min_value) {
        value = min_value;
    }
    if (max_value != DBL_MAX && value > max_value) {
        value = max_value;
    }
    
    return value;
}

std::vector<std::string> InputHandler::getMultipleChoice(const std::vector<std::string>& options, const std::string& prompt) {
    std::vector<std::string> selected;
    
    if (options.empty()) {
        return selected;
    }
    
    std::cout << prompt << std::endl;
    std::cout << "Available options:" << std::endl;
    
    for (size_t i = 0; i < options.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << options[i] << std::endl;
    }
    
    std::cout << "Enter choices (comma-separated numbers, or 'all' for all): ";
    
    std::string input = getLineInput();
    
    if (utils::toLower(input) == "all") {
        return options;
    }
    
    std::vector<std::string> choices = utils::split(input, ",");
    
    for (const std::string& choice : choices) {
        int index = utils::toInt(utils::trim(choice));
        if (index >= 1 && index <= static_cast<int>(options.size())) {
            selected.push_back(options[index - 1]);
        }
    }
    
    return selected;
}

bool InputHandler::waitForKeyPress(const std::string& message) {
    if (!message.empty()) {
        std::cout << message << std::endl;
    }
    
    getKeyInput();
    return true;
}

void InputHandler::flushInput() {
    // Clear any pending input
#ifdef _WIN32
    while (_kbhit()) {
        _getch();
    }
#else
    tcflush(STDIN_FILENO, TCIFLUSH);
#endif
}
