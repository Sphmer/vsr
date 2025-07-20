#include "config_manager.h"
#include "utils.h"
#include <iostream>
#include <fstream>
#include <algorithm>

ConfigManager::ConfigManager(const std::string& config_dir) : config_dir_(config_dir) {
    // Create config directory if it doesn't exist
    if (!utils::directoryExists(config_dir_.string())) {
        utils::createDirectory(config_dir_.string());
    }
    
    utils::log(utils::LogLevel::DEBUG, "ConfigManager initialized");
}

bool ConfigManager::configExists(const std::string& filename) const {
    std::string config_file = getConfigFilePath(filename);
    return utils::fileExists(config_file);
}

bool ConfigManager::loadConfig(const std::string& filename) {
    try {
        std::string config_file = getConfigFilePath(filename);
        
        if (!utils::fileExists(config_file)) {
            utils::log(utils::LogLevel::INFO, "Config file not found: " + config_file);
            return false;
        }
        
        std::string content = utils::readFile(config_file);
        nlohmann::json config_json = nlohmann::json::parse(content);
        
        preferences_.clear();
        
        for (auto it = config_json.begin(); it != config_json.end(); ++it) {
            std::string set_name = it.key();
            const auto& pref_json = it.value();
            
            DataSetPreference preference;
            preference.view_type = pref_json.value("view_type", "table");
            preference.slide_number = pref_json.value("slide_number", 1);
            
            if (pref_json.contains("selected_columns")) {
                for (const auto& col : pref_json["selected_columns"]) {
                    preference.selected_columns.push_back(col.get<std::string>());
                }
            }
            
            preferences_[set_name] = preference;
        }
        
        utils::log(utils::LogLevel::INFO, "Loaded config for " + std::to_string(preferences_.size()) + " data sets");
        return true;
        
    } catch (const std::exception& e) {
        utils::log(utils::LogLevel::ERROR_LEVEL, "Error loading config: " + std::string(e.what()));
        return false;
    }
}

bool ConfigManager::saveConfig(const std::string& filename, const std::map<std::string, DataSetPreference>& preferences) {
    try {
        std::string config_file = getConfigFilePath(filename);
        
        nlohmann::json config_json;
        
        for (const auto& [set_name, preference] : preferences) {
            nlohmann::json pref_json;
            pref_json["view_type"] = preference.view_type;
            pref_json["slide_number"] = preference.slide_number;
            pref_json["selected_columns"] = preference.selected_columns;
            
            config_json[set_name] = pref_json;
        }
        
        std::string content = config_json.dump(2); // Pretty print with 2 spaces
        
        if (!utils::writeFile(config_file, content)) {
            utils::log(utils::LogLevel::ERROR_LEVEL, "Failed to write config file: " + config_file);
            return false;
        }
        
        utils::log(utils::LogLevel::INFO, "Saved config to: " + config_file);
        return true;
        
    } catch (const std::exception& e) {
        utils::log(utils::LogLevel::ERROR_LEVEL, "Error saving config: " + std::string(e.what()));
        return false;
    }
}

bool ConfigManager::deleteConfig(const std::string& filename) {
    try {
        std::string config_file = getConfigFilePath(filename);
        
        if (utils::fileExists(config_file)) {
            std::filesystem::remove(config_file);
            utils::log(utils::LogLevel::INFO, "Deleted config file: " + config_file);
            return true;
        }
        
        return false;
        
    } catch (const std::exception& e) {
        utils::log(utils::LogLevel::ERROR_LEVEL, "Error deleting config: " + std::string(e.what()));
        return false;
    }
}

std::map<std::string, DataSetPreference> ConfigManager::askRepresentationPreferences(const std::map<std::string, DataSet>& data_sets) {
    std::map<std::string, DataSetPreference> preferences;
    
    std::cout << "\n=== VSR Configuration Setup ===" << std::endl;
    std::cout << "Configure how each data set should be displayed.\n" << std::endl;
    
    int slide_counter = 1;
    
    for (const auto& [set_name, data_set] : data_sets) {
        std::cout << "Configuring data set: " << set_name << std::endl;
        std::cout << "Rows: " << data_set.rows.size() << std::endl;
        
        // Get available columns
        std::vector<std::string> available_columns;
        if (!data_set.rows.empty()) {
            for (const auto& [key, value] : data_set.rows[0]) {
                available_columns.push_back(key);
            }
        }
        
        std::cout << "Available columns: " << utils::join(available_columns, ", ") << std::endl;
        
        DataSetPreference preference;
        
        // Ask for view type
        preference.view_type = askViewType();
        
        // Ask for slide number
        preference.slide_number = askSlideNumber(slide_counter);
        
        // Ask for column selection
        preference.selected_columns = askColumnSelection(available_columns);
        
        preferences[set_name] = preference;
        
        std::cout << std::endl;
        slide_counter++;
    }
    
    preferences_ = preferences;
    return preferences;
}

std::string ConfigManager::askViewType() {
    std::cout << "\nSelect view type:" << std::endl;
    std::cout << "1. Table view" << std::endl;
    std::cout << "2. Bar chart view" << std::endl;
    std::cout << "3. Tree view" << std::endl;
    std::cout << "4. Mixed view (default)" << std::endl;
    std::cout << "Enter choice (1-4): ";
    
    std::string input;
    std::getline(std::cin, input);
    input = utils::trim(input);
    
    if (input == "1") return "table";
    if (input == "2") return "bars";
    if (input == "3") return "tree";
    if (input == "4" || input.empty()) return "mixed";
    
    return "mixed"; // Default
}

int ConfigManager::askSlideNumber(int suggested_slide) {
    std::cout << "\nSlide number (default: " << suggested_slide << "): ";
    
    std::string input;
    std::getline(std::cin, input);
    input = utils::trim(input);
    
    if (input.empty()) {
        return suggested_slide;
    }
    
    int slide_num = utils::toInt(input);
    return (slide_num > 0) ? slide_num : suggested_slide;
}

std::vector<std::string> ConfigManager::askColumnSelection(const std::vector<std::string>& available_columns) {
    if (available_columns.empty()) {
        return {};
    }
    
    std::cout << "\nColumn selection:" << std::endl;
    std::cout << "Available columns:" << std::endl;
    
    for (size_t i = 0; i < available_columns.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << available_columns[i] << std::endl;
    }
    
    std::cout << "\nSelect columns (comma-separated numbers, or 'all' for all columns): ";
    
    std::string input;
    std::getline(std::cin, input);
    input = utils::trim(input);
    
    if (input.empty() || utils::toLower(input) == "all") {
        return available_columns; // Return all columns
    }
    
    std::vector<std::string> selected_columns;
    std::vector<std::string> selections = utils::split(input, ",");
    
    for (const std::string& selection : selections) {
        std::string trimmed = utils::trim(selection);
        int index = utils::toInt(trimmed);
        
        if (index >= 1 && index <= static_cast<int>(available_columns.size())) {
            selected_columns.push_back(available_columns[index - 1]);
        }
    }
    
    if (selected_columns.empty()) {
        std::cout << "No valid columns selected, using all columns." << std::endl;
        return available_columns;
    }
    
    return selected_columns;
}

// getPreferences() and setPreferences() are inline methods defined in header

std::vector<std::string> ConfigManager::listConfigs() {
    std::vector<std::string> configs;
    
    try {
        if (utils::directoryExists(config_dir_.string())) {
            configs = utils::listFiles(config_dir_.string(), ".json");
        }
    } catch (const std::exception& e) {
        utils::log(utils::LogLevel::ERROR_LEVEL, "Error listing configs: " + std::string(e.what()));
    }
    
    return configs;
}

std::string ConfigManager::getConfigInfo(const std::string& filename) {
    try {
        std::string config_file = getConfigFilePath(filename);
        
        if (!utils::fileExists(config_file)) {
            return "Config file not found";
        }
        
        std::string content = utils::readFile(config_file);
        nlohmann::json config_json = nlohmann::json::parse(content);
        
        std::ostringstream info;
        info << "Configuration for: " << filename << "\n";
        info << "Data sets: " << config_json.size() << "\n";
        
        for (auto it = config_json.begin(); it != config_json.end(); ++it) {
            const auto& pref = it.value();
            info << "  " << it.key() << ": " << pref.value("view_type", "mixed") 
                 << " (slide " << pref.value("slide_number", 1) << ")\n";
        }
        
        return info.str();
        
    } catch (const std::exception& e) {
        return "Error reading config: " + std::string(e.what());
    }
}

std::string ConfigManager::getConfigFilePath(const std::string& filename) const {
    std::string hash = utils::calculateFileHash(filename);
    return (config_dir_ / (hash + ".json")).string();
}

bool ConfigManager::validatePreferences(const std::map<std::string, DataSetPreference>& preferences) {
    for (const auto& [set_name, preference] : preferences) {
        // Validate view type
        if (preference.view_type != "table" && 
            preference.view_type != "bars" && 
            preference.view_type != "tree" && 
            preference.view_type != "mixed") {
            utils::log(utils::LogLevel::WARNING, "Invalid view type for " + set_name + ": " + preference.view_type);
            return false;
        }
        
        // Validate slide number
        if (preference.slide_number < 1) {
            utils::log(utils::LogLevel::WARNING, "Invalid slide number for " + set_name + ": " + std::to_string(preference.slide_number));
            return false;
        }
    }
    
    return true;
}
