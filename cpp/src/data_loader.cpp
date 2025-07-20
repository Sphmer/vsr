#include "data_loader.h"
#include "utils.h"
#include <fstream>
#include <sstream>
#include <algorithm>

DataLoader::DataLoader() {
    utils::log(utils::LogLevel::DEBUG, "DataLoader initialized");
}

bool DataLoader::loadFromFile(const std::string& filename) {
    filename_ = filename;
    data_sets_.clear();
    
    try {
        std::string extension = utils::getFileExtension(filename);
        
        if (extension == ".json") {
            return loadJSON(filename);
        } else if (extension == ".csv") {
            return loadCSV(filename);
        } else {
            throw utils::VSRException("Unsupported file format: " + extension);
        }
        
    } catch (const std::exception& e) {
        utils::log(utils::LogLevel::ERROR_LEVEL, "Error loading file: " + std::string(e.what()));
        return false;
    }
}

bool DataLoader::loadJSON(const std::string& filename) {
    try {
        std::string content = utils::readFile(filename);
        
        if (!utils::isValidJSON(content)) {
            throw utils::VSRException("Invalid JSON format");
        }
        
        nlohmann::json json_data = nlohmann::json::parse(content);
        
        // Identify data sets within the JSON
        identifyJSONDataSets(json_data);
        
        utils::log(utils::LogLevel::INFO, "Successfully loaded JSON file with " + 
                  std::to_string(data_sets_.size()) + " data sets");
        return true;
        
    } catch (const nlohmann::json::parse_error& e) {
        utils::log(utils::LogLevel::ERROR_LEVEL, "JSON parse error: " + std::string(e.what()));
        return false;
    } catch (const std::exception& e) {
        utils::log(utils::LogLevel::ERROR_LEVEL, "Error loading JSON: " + std::string(e.what()));
        return false;
    }
}

bool DataLoader::loadCSV(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw utils::VSRException("Cannot open CSV file: " + filename);
        }
        
        std::vector<std::vector<std::string>> csv_data;
        std::string line;
        
        // Read all lines
        while (std::getline(file, line)) {
            if (!line.empty()) {
                csv_data.push_back(parseCSVLine(line));
            }
        }
        
        if (csv_data.empty()) {
            throw utils::VSRException("Empty CSV file");
        }
        
        // Convert CSV to data set
        convertCSVToDataSet(csv_data);
        
        utils::log(utils::LogLevel::INFO, "Successfully loaded CSV file with " + 
                  std::to_string(csv_data.size()) + " rows");
        return true;
        
    } catch (const std::exception& e) {
        utils::log(utils::LogLevel::ERROR_LEVEL, "Error loading CSV: " + std::string(e.what()));
        return false;
    }
}

void DataLoader::identifyJSONDataSets(const nlohmann::json& json_data) {
    if (json_data.is_array()) {
        // Array of objects - single data set
        DataSet data_set;
        data_set.name = "main";
        data_set.type = DataSetType::ARRAY;
        
        for (const auto& item : json_data) {
            if (item.is_object()) {
                DataRow row;
                for (auto it = item.begin(); it != item.end(); ++it) {
                    row[it.key()] = convertJSONValue(it.value());
                }
                data_set.rows.push_back(row);
            }
        }
        
        data_sets_["main"] = data_set;
        
    } else if (json_data.is_object()) {
        // Check if it's a nested structure with multiple data sets
        bool has_nested_arrays = false;
        
        for (auto it = json_data.begin(); it != json_data.end(); ++it) {
            if (it.value().is_array() && !it.value().empty() && it.value()[0].is_object()) {
                has_nested_arrays = true;
                
                // Create data set for this array
                DataSet data_set;
                data_set.name = it.key();
                data_set.type = DataSetType::NESTED;
                
                for (const auto& item : it.value()) {
                    if (item.is_object()) {
                        DataRow row;
                        for (auto item_it = item.begin(); item_it != item.end(); ++item_it) {
                            row[item_it.key()] = convertJSONValue(item_it.value());
                        }
                        data_set.rows.push_back(row);
                    }
                }
                
                data_sets_[it.key()] = data_set;
            }
        }
        
        // If no nested arrays found, treat as flat key-value pairs
        if (!has_nested_arrays) {
            DataSet data_set;
            data_set.name = "main";
            data_set.type = DataSetType::FLAT;
            
            DataRow row;
            for (auto it = json_data.begin(); it != json_data.end(); ++it) {
                row[it.key()] = convertJSONValue(it.value());
            }
            data_set.rows.push_back(row);
            
            data_sets_["main"] = data_set;
        }
    }
}

void DataLoader::convertCSVToDataSet(const std::vector<std::vector<std::string>>& csv_data) {
    if (csv_data.empty()) return;
    
    DataSet data_set;
    data_set.name = "main";
    data_set.type = DataSetType::CSV;
    
    // First row is headers
    const auto& headers = csv_data[0];
    
    // Process data rows
    for (size_t i = 1; i < csv_data.size(); ++i) {
        const auto& row_data = csv_data[i];
        
        DataRow row;
        for (size_t j = 0; j < headers.size() && j < row_data.size(); ++j) {
            std::string value = utils::trim(row_data[j]);
            row[headers[j]] = utils::stringToAny(value);
        }
        
        data_set.rows.push_back(row);
    }
    
    data_sets_["main"] = data_set;
}

std::vector<std::string> DataLoader::parseCSVLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool in_quotes = false;
    
    for (size_t i = 0; i < line.length(); ++i) {
        char c = line[i];
        
        if (c == '"') {
            in_quotes = !in_quotes;
        } else if (c == ',' && !in_quotes) {
            fields.push_back(utils::trim(field));
            field.clear();
        } else {
            field += c;
        }
    }
    
    // Add the last field
    fields.push_back(utils::trim(field));
    
    return fields;
}

std::any DataLoader::convertJSONValue(const nlohmann::json& value) {
    if (value.is_string()) {
        return std::any(value.get<std::string>());
    } else if (value.is_number_integer()) {
        return std::any(value.get<int>());
    } else if (value.is_number_float()) {
        return std::any(value.get<double>());
    } else if (value.is_boolean()) {
        return std::any(value.get<bool>());
    } else if (value.is_null()) {
        return std::any(std::string("null"));
    } else {
        // For complex types, convert to string representation
        return std::any(value.dump());
    }
}

std::vector<std::string> DataLoader::getDataSetNames() const {
    std::vector<std::string> names;
    for (const auto& [name, data_set] : data_sets_) {
        names.push_back(name);
    }
    return names;
}

DataSet DataLoader::getDataSet(const std::string& name) const {
    auto it = data_sets_.find(name);
    if (it != data_sets_.end()) {
        return it->second;
    }
    
    throw utils::VSRException("Data set not found: " + name);
}

std::map<std::string, DataSet> DataLoader::getDataSets() const {
    return data_sets_;
}

std::vector<std::string> DataLoader::getColumnNames(const std::string& data_set_name) const {
    auto it = data_sets_.find(data_set_name);
    if (it == data_sets_.end()) {
        return {};
    }
    
    const DataSet& data_set = it->second;
    if (data_set.rows.empty()) {
        return {};
    }
    
    // For CSV files, preserve the original column order from the first row
    if (data_set.type == DataSetType::CSV) {
        std::vector<std::string> columns;
        for (const auto& [key, value] : data_set.rows[0]) {
            columns.push_back(key);
        }
        return columns;
    }
    
    // For other types, collect all unique column names
    std::set<std::string> unique_columns;
    for (const auto& row : data_set.rows) {
        for (const auto& [key, value] : row) {
            unique_columns.insert(key);
        }
    }
    
    return std::vector<std::string>(unique_columns.begin(), unique_columns.end());
}

size_t DataLoader::getRowCount(const std::string& data_set_name) const {
    auto it = data_sets_.find(data_set_name);
    if (it != data_sets_.end()) {
        return it->second.rows.size();
    }
    return 0;
}

bool DataLoader::hasDataSet(const std::string& name) const {
    return data_sets_.find(name) != data_sets_.end();
}

std::string DataLoader::getDataSetInfo(const std::string& name) const {
    auto it = data_sets_.find(name);
    if (it == data_sets_.end()) {
        return "Data set not found";
    }
    
    const DataSet& data_set = it->second;
    std::ostringstream info;
    
    info << "Data Set: " << name << "\n";
    info << "Type: ";
    
    switch (data_set.type) {
        case DataSetType::FLAT: info << "Flat"; break;
        case DataSetType::NESTED: info << "Nested"; break;
        case DataSetType::ARRAY: info << "Array"; break;
        case DataSetType::CSV: info << "CSV"; break;
    }
    
    info << "\n";
    info << "Rows: " << data_set.rows.size() << "\n";
    info << "Columns: " << getColumnNames(name).size();
    
    return info.str();
}
