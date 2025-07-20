#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <filesystem>
#include "data_loader.h"
#include "json.hpp"

using json = nlohmann::json;

class ConfigManager {
public:
    explicit ConfigManager(const std::string& config_dir = "rep_saved");
    ~ConfigManager() = default;

    // Configuration management
    bool loadConfig(const std::string& filepath);
    bool saveConfig(const std::string& filepath, const std::map<std::string, DataSetPreference>& preferences);
    bool deleteConfig(const std::string& filepath);
    bool configExists(const std::string& filepath) const;

    // Data access
    const std::map<std::string, DataSetPreference>& getPreferences() const { return preferences_; }
    void setPreferences(const std::map<std::string, DataSetPreference>& preferences) { preferences_ = preferences; }

    // User interaction methods
    std::map<std::string, DataSetPreference> askRepresentationPreferences(
        const std::map<std::string, DataSet>& data_sets
    );

    DataSetPreference configureDataSet(
        const std::string& set_name,
        const DataSet& data_set,
        int current_index,
        int total_count,
        const std::map<std::string, DataSetPreference>& existing_preferences,
        const std::vector<std::string>& data_set_names
    );

    // Column and field selection
    std::vector<std::string> askColumnSelection(
        const std::string& set_name,
        const std::vector<std::string>& columns
    );

    std::string askBarFieldSelection(
        const std::string& set_name,
        const std::vector<std::string>& numeric_fields
    );

    int askSlideSelection(
        const std::string& set_name,
        const std::map<std::string, DataSetPreference>& existing_preferences
    );

    // Display methods
    void showConfigurationSummary(const std::map<std::string, DataSetPreference>& preferences);
    std::string createProgressBar(
        int current_index,
        int total_count,
        const std::map<std::string, DataSetPreference>& existing_preferences,
        const std::vector<std::string>& data_set_names
    );

    // Additional user interaction methods
    std::string askViewType();
    int askSlideNumber(int suggested_slide);
    std::vector<std::string> askColumnSelection(const std::vector<std::string>& available_columns);
    
    // Configuration file management
    std::vector<std::string> listConfigs();
    std::string getConfigInfo(const std::string& filename);
    std::string getConfigFilePath(const std::string& filename) const;
    bool validatePreferences(const std::map<std::string, DataSetPreference>& preferences);

private:
    std::filesystem::path config_dir_;
    std::map<std::string, DataSetPreference> preferences_;

    // Helper methods
    std::string getFileHash(const std::string& filepath) const;
    std::filesystem::path getConfigPath(const std::string& filepath) const;
    json preferencesToJson(const std::map<std::string, DataSetPreference>& preferences) const;
    std::map<std::string, DataSetPreference> jsonToPreferences(const json& config_json) const;
    
    // Utility methods
    void ensureConfigDirectory();
    std::string getCurrentTimestamp() const;
};
