#include "vsr_app.h"
#include "utils.h"
#include <iostream>
#include <algorithm>

VSRApp::VSRApp(const std::string& filename) 
    : filename_(filename)
    , view_mode_("mixed")
    , scroll_offset_(0)
    , terminal_width_(80)
    , terminal_height_(24)
    , max_display_rows_(20)
    , current_slide_(1)
    , total_slides_(1)
    , config_loaded_(false)
    , isRunning_(false) {
    
    // Initialize components
    data_loader_ = std::make_unique<DataLoader>();
    data_processor_ = std::make_unique<DataProcessor>();
    config_manager_ = std::make_unique<ConfigManager>();
    display_manager_ = std::make_unique<DisplayManager>();
    input_handler_ = std::make_unique<InputHandler>();
}

bool VSRApp::initialize() {
    try {
        utils::log(utils::LogLevel::INFO, "Initializing VSR application...");
        
        // Get terminal size
        getTerminalSize();
        
        // Load data from file
        if (!loadData()) {
            utils::log(utils::LogLevel::ERROR_LEVEL, "Failed to load data from file: " + filename_);
            return false;
        }
        
        // Process data into data sets
        processData();
        identifyDataSets();
        
        // Load or create configuration
        if (!loadOrCreateConfig()) {
            utils::log(utils::LogLevel::ERROR_LEVEL, "Failed to load or create configuration");
            return false;
        }
        
        // Organize slides
        organizeSlides();
        
        utils::log(utils::LogLevel::INFO, "VSR application initialized successfully");
        return true;
        
    } catch (const std::exception& e) {
        utils::log(utils::LogLevel::ERROR_LEVEL, "Exception during initialization: " + std::string(e.what()));
        return false;
    }
}

void VSRApp::run() {
    isRunning_ = true;
    
    utils::log(utils::LogLevel::INFO, "Starting VSR main loop");
    
    // Clear screen and show initial display
    clearScreen();
    displayScreen();
    
    // Main application loop
    while (isRunning_) {
        try {
            // Get user input
            std::string key = input_handler_->getKeyInput();
            
            // Handle the input
            if (!handleInput(key)) {
                break; // User requested quit
            }
            
            // Update display
            displayScreen();
            
        } catch (const std::exception& e) {
            utils::log(utils::LogLevel::ERROR_LEVEL, "Exception in main loop: " + std::string(e.what()));
            std::cout << "Error: " << e.what() << std::endl;
            std::cout << "Press any key to continue..." << std::endl;
            input_handler_->getKeyInput();
        }
    }
    
    utils::log(utils::LogLevel::INFO, "VSR main loop ended");
}

void VSRApp::shutdown() {
    isRunning_ = false;
    clearScreen();
    utils::log(utils::LogLevel::INFO, "VSR application shutdown complete");
}

bool VSRApp::loadData() {
    try {
        return data_loader_->loadFromFile(filename_);
    } catch (const std::exception& e) {
        utils::log(utils::LogLevel::ERROR_LEVEL, "Error loading data: " + std::string(e.what()));
        return false;
    }
}

void VSRApp::processData() {
    data_sets_ = data_loader_->getDataSets();
    utils::log(utils::LogLevel::INFO, "Loaded " + std::to_string(data_sets_.size()) + " data sets");
}

void VSRApp::identifyDataSets() {
    // Data sets are already identified by the data loader
    // This method exists for compatibility with the Python version
    utils::log(utils::LogLevel::INFO, "Data sets identified: " + std::to_string(data_sets_.size()));
}

bool VSRApp::loadOrCreateConfig() {
    try {
        if (config_manager_->configExists(filename_)) {
            utils::log(utils::LogLevel::INFO, "Loading existing configuration");
            config_loaded_ = config_manager_->loadConfig(filename_);
            
            if (config_loaded_) {
                data_set_preferences_ = config_manager_->getPreferences();
                return true;
            }
        }
        
        // No existing config or failed to load, ask user for preferences
        utils::log(utils::LogLevel::INFO, "Creating new configuration");
        askRepresentationPreferences();
        
        // Save the new configuration
        config_manager_->saveConfig(filename_, data_set_preferences_);
        config_loaded_ = true;
        
        return true;
        
    } catch (const std::exception& e) {
        utils::log(utils::LogLevel::ERROR_LEVEL, "Error with configuration: " + std::string(e.what()));
        return false;
    }
}

void VSRApp::askRepresentationPreferences() {
    data_set_preferences_ = config_manager_->askRepresentationPreferences(data_sets_);
}

void VSRApp::reconfigureRepresentations() {
    try {
        clearScreen();
        std::cout << "Reconfiguring representation preferences...\n" << std::endl;
        
        askRepresentationPreferences();
        
        // Save the updated configuration
        config_manager_->saveConfig(filename_, data_set_preferences_);
        
        // Reprocess data with new preferences
        processed_data_ = data_processor_->processDataSets(data_sets_, data_set_preferences_);
        
        // Reorganize slides
        organizeSlides();
        updateProcessedDataForCurrentSlide();
        
        std::cout << "Configuration updated successfully!" << std::endl;
        std::cout << "Press any key to continue..." << std::endl;
        input_handler_->getKeyInput();
        
    } catch (const std::exception& e) {
        std::cout << "Error reconfiguring: " << e.what() << std::endl;
        std::cout << "Press any key to continue..." << std::endl;
        input_handler_->getKeyInput();
    }
}

void VSRApp::displayScreen() {
    clearScreen();
    
    // Update processed data for current slide
    updateProcessedDataForCurrentSlide();
    
    // Display based on current view mode
    if (view_mode_ == "table") {
        createTableView();
    } else if (view_mode_ == "bars") {
        createBarView();
    } else if (view_mode_ == "tree") {
        createTreeView();
    } else {
        createMixedView();
    }
    
    // Display slide information
    display_manager_->displaySlideInfo(current_slide_, total_slides_);
    
    // Display help information
    std::cout << "\nControls: [↑/↓] Scroll | [←/→] Slides | [t] Table | [b] Bars | [m] Mixed | [r] Reconfigure | [h] Help | [q] Quit" << std::endl;
}

void VSRApp::createTableView() {
    display_manager_->displayTableView(processed_data_, scroll_offset_, max_display_rows_);
}

void VSRApp::createBarView() {
    display_manager_->displayBarView(processed_data_, scroll_offset_, max_display_rows_);
}

void VSRApp::createTreeView() {
    display_manager_->displayTreeView(processed_data_, scroll_offset_, max_display_rows_);
}

void VSRApp::createMixedView() {
    display_manager_->displayMixedView(processed_data_, scroll_offset_, max_display_rows_);
}

void VSRApp::showHelp() {
    clearScreen();
    display_manager_->displayHelp();
    
    std::cout << "\nPress any key to return..." << std::endl;
    input_handler_->getKeyInput();
}

void VSRApp::clearScreen() {
    display_manager_->clearScreen();
}

bool VSRApp::handleInput(const std::string& key) {
    if (key == "q" || key == "quit") {
        return false; // Quit application
    }
    
    if (key == "h" || key == "help") {
        showHelp();
        return true;
    }
    
    if (key == "r" || key == "reconfigure") {
        reconfigureRepresentations();
        return true;
    }
    
    // View mode changes
    if (key == "t" || key == "table") {
        view_mode_ = "table";
        return true;
    }
    
    if (key == "b" || key == "bars") {
        view_mode_ = "bars";
        return true;
    }
    
    if (key == "m" || key == "mixed") {
        view_mode_ = "mixed";
        return true;
    }
    
    // Navigation
    if (key == "up" || key == "k") {
        if (scroll_offset_ > 0) {
            scroll_offset_--;
        }
        return true;
    }
    
    if (key == "down" || key == "j") {
        scroll_offset_++;
        return true;
    }
    
    if (key == "left" || key == "h") {
        if (current_slide_ > 1) {
            current_slide_--;
            scroll_offset_ = 0;
        }
        return true;
    }
    
    if (key == "right" || key == "l") {
        if (current_slide_ < total_slides_) {
            current_slide_++;
            scroll_offset_ = 0;
        }
        return true;
    }
    
    // Page up/down
    if (key == "pageup") {
        scroll_offset_ = std::max(0, scroll_offset_ - max_display_rows_);
        return true;
    }
    
    if (key == "pagedown") {
        scroll_offset_ += max_display_rows_;
        return true;
    }
    
    // Home/End
    if (key == "home") {
        scroll_offset_ = 0;
        return true;
    }
    
    return true; // Continue running
}

void VSRApp::updateProcessedDataForCurrentSlide() {
    // Process all data sets with current preferences
    auto all_processed = data_processor_->processDataSets(data_sets_, data_set_preferences_);
    
    // Filter to only show data sets for current slide
    processed_data_.clear();
    
    if (slides_.find(current_slide_) != slides_.end()) {
        const auto& slide_data_sets = slides_[current_slide_];
        
        for (const auto& processed : all_processed) {
            if (std::find(slide_data_sets.begin(), slide_data_sets.end(), processed.set_name) != slide_data_sets.end()) {
                processed_data_.push_back(processed);
            }
        }
    }
}

void VSRApp::getTerminalSize() {
    auto size = utils::getConsoleSize();
    terminal_width_ = size.first;
    terminal_height_ = size.second;
    max_display_rows_ = std::max(5, terminal_height_ - 10); // Leave space for UI elements
}

std::string VSRApp::showFileSelectionMenu() {
    return display_manager_->showFileSelectionMenu({filename_});
}

void VSRApp::organizeSlides() {
    slides_.clear();
    total_slides_ = 1;
    
    // Group data sets by slide number
    for (const auto& [set_name, preference] : data_set_preferences_) {
        int slide_num = preference.slide_number;
        if (slide_num < 1) slide_num = 1;
        
        slides_[slide_num].push_back(set_name);
        total_slides_ = std::max(total_slides_, slide_num);
    }
    
    // Ensure current slide is valid
    if (current_slide_ > total_slides_) {
        current_slide_ = 1;
    }
    
    utils::log(utils::LogLevel::INFO, "Organized " + std::to_string(total_slides_) + " slides");
}
