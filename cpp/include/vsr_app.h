#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <unordered_map>
#include <atomic>
#include <csignal>
#include "data_loader.h"
#include "data_processor.h"
#include "config_manager.h"
#include "display_manager.h"
#include "input_handler.h"

class VSRApp {
public:
    explicit VSRApp(const std::string& filename);
    ~VSRApp() = default;

    // Main application methods
    bool initialize();
    void run();
    void shutdown();

    // Data management
    bool loadData();
    void processData();
    void identifyDataSets();

    // Configuration management
    bool loadOrCreateConfig();
    void reconfigureRepresentations();
    void askRepresentationPreferences();

    // Display methods
    void displayScreen();
    void createTableView();
    void createBarView();
    void createTreeView();
    void createMixedView();
    void showHelp();
    void clearScreen();

    // Input handling
    bool handleInput(const std::string& key);
    void updateProcessedDataForCurrentSlide();

    // Utility methods
    void getTerminalSize();
    std::string showFileSelectionMenu();

private:
    // Core data members
    std::string filename_;
    std::unique_ptr<DataLoader> data_loader_;
    std::unique_ptr<DataProcessor> data_processor_;
    std::unique_ptr<ConfigManager> config_manager_;
    std::unique_ptr<DisplayManager> display_manager_;
    std::unique_ptr<InputHandler> input_handler_;

    // Application state
    std::map<std::string, DataSet> data_sets_;
    std::vector<ProcessedData> processed_data_;
    std::string view_mode_;  // "table", "bars", "tree", "mixed"
    int scroll_offset_;
    int terminal_width_;
    int terminal_height_;
    int max_display_rows_;

    // Slides functionality
    std::map<int, std::vector<std::string>> slides_;
    int current_slide_;
    int total_slides_;
    std::map<std::string, DataSetPreference> data_set_preferences_;

    // Configuration
    std::map<std::string, std::any> current_config_;
    bool config_loaded_;

    // Terminal resize detection
    static std::atomic<bool> terminal_resized_;
    int last_terminal_width_;
    int last_terminal_height_;

    // Helper methods
    void organizeSlides();
    void updateSlideData();
    bool isRunning_;

    // Resize handling
    void setupResizeHandler();
    static void handleResizeSignal(int signum);
    bool checkAndHandleResize();
};
