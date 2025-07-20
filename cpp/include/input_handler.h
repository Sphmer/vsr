#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <climits>
#include <cfloat>

class InputHandler {
public:
    InputHandler();
    ~InputHandler() = default;

    // Key input methods
    std::string getKeyInput();
    std::string getLineInput();
    std::string normalizeKey(const std::string& raw_key);
    std::string normalizeInput(const std::string& input);
    
    // Input validation
    bool isValidKey(const std::string& key) const;
    bool isNavigationKey(const std::string& key) const;
    bool isActionKey(const std::string& key) const;
    bool isQuitKey(const std::string& key) const;

    // Key mapping
    std::string mapSpecialKeys(const std::string& key) const;
    
    // Input validation and mapping
    bool validateInput(const std::string& input, const std::vector<std::string>& valid_inputs);
    std::string mapInput(const std::string& input, const std::map<std::string, std::string>& input_map);
    
    // Interactive input methods
    int getMenuSelection(const std::vector<std::string>& options);
    int getMenuSelection(
        const std::vector<std::string>& options,
        int current_selection = 0,
        const std::string& prompt = "Select option:"
    );
    
    bool getConfirmation(const std::string& prompt = "Continue? (y/n): ");
    bool confirmAction(const std::string& message);
    std::string getTextInput(const std::string& prompt = "Enter text: ");
    std::string getStringInput(const std::string& prompt, const std::string& default_value = "");
    int getIntInput(const std::string& prompt, int default_value = 0, int min_value = INT_MIN, int max_value = INT_MAX);
    double getDoubleInput(const std::string& prompt, double default_value = 0.0, double min_value = -DBL_MAX, double max_value = DBL_MAX);
    
    // Multi-selection methods
    std::vector<std::string> getMultipleChoice(const std::vector<std::string>& options, const std::string& prompt);
    std::vector<int> getMultiSelection(
        const std::vector<std::string>& options,
        const std::vector<int>& initial_selection = {},
        const std::string& prompt = "Select options (space to toggle, enter to confirm):"
    );
    
    // Utility methods
    bool waitForKeyPress(const std::string& message = "Press any key to continue...");
    void flushInput();

private:
    // Platform-specific input methods
    std::string getRawKeyInput();
    void enableRawMode();
    void disableRawMode();
    
    // Key code mappings
    std::map<std::string, std::string> special_key_map_;
    
    // Helper methods
    void initializeKeyMappings();
    bool isEscapeSequence(const std::string& input) const;
    std::string parseEscapeSequence(const std::string& sequence) const;
    
    // State management
    bool raw_mode_enabled_;
    void setupRawMode();
    void restoreTerminalMode();
};
