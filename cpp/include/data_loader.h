#pragma once

#include <string>
#include <vector>
#include <map>
#include <any>
#include <memory>
#include <set>
#include "json.hpp"

using json = nlohmann::json;

// Data structures
enum class DataSetType {
    FLAT,
    NESTED,
    ARRAY,
    CSV
};

using DataRow = std::map<std::string, std::any>;

struct DataSet {
    std::string name;
    std::vector<std::map<std::string, std::any>> data;
    std::vector<DataRow> rows;  // For compatibility with source code
    std::vector<std::string> columns;
    std::vector<std::string> numeric_fields;
    DataSetType type = DataSetType::FLAT;
};

// Statistics structure for column analysis
struct ColumnStatistics {
    bool is_numeric = false;
    double min_value = 0.0;
    double max_value = 0.0;
    double sum_value = 0.0;
    double avg_value = 0.0;
    size_t count = 0;
};

struct ProcessedData {
    std::string set_name;
    std::vector<std::map<std::string, std::string>> rows;
    std::vector<std::string> selected_columns;
    std::vector<std::string> columns;  // All available columns
    std::string display_type; // "table", "bars", "tree"
    std::string view_type;    // View type for mixed displays
    std::string bar_field;
    int slide_number;
    std::map<std::string, ColumnStatistics> column_stats;  // Column statistics
};

// Alias for compatibility with source files
using ProcessedDataSet = ProcessedData;
using ProcessedRow = std::map<std::string, std::string>;

struct DataSetPreference {
    std::string display_type;
    std::vector<std::string> selected_columns;
    std::string bar_field;
    int slide_number;
    std::vector<std::string> manual_column_order;
    bool use_manual_order;
    std::string view_type;  // View type for mixed displays
};

class DataLoader {
public:
    DataLoader();
    ~DataLoader() = default;

    // Main loading methods
    bool loadFromFile(const std::string& filename);
    bool loadJSON(const std::string& filename);
    bool loadCSV(const std::string& filename);

    // Data access methods
    std::vector<std::string> getDataSetNames() const;
    DataSet getDataSet(const std::string& name) const;
    std::map<std::string, DataSet> getDataSets() const;
    std::vector<std::string> getColumnNames(const std::string& data_set_name) const;
    size_t getRowCount(const std::string& data_set_name) const;
    bool hasDataSet(const std::string& name) const;
    std::string getDataSetInfo(const std::string& name) const;
    
    // Utility methods
    bool isJSONFile(const std::string& filepath) const;
    bool isCSVFile(const std::string& filepath) const;
    std::string getFileExtension(const std::string& filepath) const;

private:
    std::string filename_;
    std::map<std::string, DataSet> data_sets_;

    // Helper methods
    void identifyJSONDataSets(const nlohmann::json& json_data);
    void convertCSVToDataSet(const std::vector<std::vector<std::string>>& csv_data);
    std::vector<std::string> parseCSVLine(const std::string& line);
    std::any convertJSONValue(const nlohmann::json& value);
    void processJSONDataSet(const std::string& name, const json& data);
    void processCSVDataSet(const std::vector<std::map<std::string, std::string>>& csv_data);
    std::vector<std::string> getNumericFields(const std::vector<std::map<std::string, std::any>>& data);
    std::vector<std::string> getNumericFieldsFromDict(const std::map<std::string, std::any>& data);
    bool isNumeric(const std::string& value) const;
    std::any convertValue(const std::string& value) const;
};
