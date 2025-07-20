#include "data_processor.h"
#include "utils.h"
#include <algorithm>
#include <numeric>

std::vector<ProcessedDataSet> DataProcessor::processDataSets(
    const std::map<std::string, DataSet>& data_sets,
    const std::map<std::string, DataSetPreference>& preferences) {
    
    std::vector<ProcessedDataSet> processed_sets;
    
    for (const auto& [set_name, data_set] : data_sets) {
        auto pref_it = preferences.find(set_name);
        if (pref_it != preferences.end()) {
            ProcessedDataSet processed = processDataSet(data_set, pref_it->second);
            processed.set_name = set_name;
            processed_sets.push_back(processed);
        }
    }
    
    return processed_sets;
}

ProcessedDataSet DataProcessor::processDataSet(const DataSet& data_set, const DataSetPreference& preference) {
    ProcessedDataSet processed;
    processed.set_name = data_set.name;
    processed.view_type = preference.view_type;
    processed.slide_number = preference.slide_number;
    
    // Filter columns based on preference
    std::vector<std::string> selected_columns;
    if (preference.selected_columns.empty()) {
        // Use all columns if none specified
        if (!data_set.rows.empty()) {
            for (const auto& [key, value] : data_set.rows[0]) {
                selected_columns.push_back(key);
            }
        }
    } else {
        selected_columns = preference.selected_columns;
    }
    
    processed.columns = selected_columns;
    
    // Process rows
    for (const auto& row : data_set.rows) {
        ProcessedRow processed_row;
        
        for (const std::string& col : selected_columns) {
            auto it = row.find(col);
            if (it != row.end()) {
                // Convert std::any to string
                try {
                    processed_row[col] = std::any_cast<std::string>(it->second);
                } catch (const std::bad_any_cast& e) {
                    processed_row[col] = "N/A";
                }
            } else {
                processed_row[col] = "N/A";
            }
        }
        
        processed.rows.push_back(processed_row);
    }
    
    // Calculate statistics if needed
    calculateStatistics(processed);
    
    return processed;
}

void DataProcessor::calculateStatistics(ProcessedDataSet& processed) {
    if (processed.rows.empty()) return;
    
    // Calculate statistics for numeric columns
    for (const std::string& col : processed.columns) {
        std::vector<double> numeric_values;
        
        for (const auto& row : processed.rows) {
            auto it = row.find(col);
            if (it != row.end()) {
                std::string str_val = utils::anyToString(it->second);
                if (utils::isNumeric(str_val)) {
                    numeric_values.push_back(utils::toDouble(str_val));
                }
            }
        }
        
        if (!numeric_values.empty()) {
            ColumnStatistics stats;
            stats.is_numeric = true;
            stats.min_value = *std::min_element(numeric_values.begin(), numeric_values.end());
            stats.max_value = *std::max_element(numeric_values.begin(), numeric_values.end());
            stats.sum_value = std::accumulate(numeric_values.begin(), numeric_values.end(), 0.0);
            stats.avg_value = stats.sum_value / numeric_values.size();
            stats.count = numeric_values.size();
            
            processed.column_stats[col] = stats;
        } else {
            ColumnStatistics stats;
            stats.is_numeric = false;
            stats.count = processed.rows.size();
            
            processed.column_stats[col] = stats;
        }
    }
}

std::vector<std::string> DataProcessor::convertDataToStrings(const ProcessedDataSet& data_set) {
    std::vector<std::string> string_data;
    
    // Add header
    std::string header = utils::join(data_set.columns, ",");
    string_data.push_back(header);
    
    // Add data rows
    for (const auto& row : data_set.rows) {
        std::vector<std::string> row_strings;
        
        for (const std::string& col : data_set.columns) {
            auto it = row.find(col);
            if (it != row.end()) {
                row_strings.push_back(utils::anyToString(it->second));
            } else {
                row_strings.push_back("N/A");
            }
        }
        
        string_data.push_back(utils::join(row_strings, ","));
    }
    
    return string_data;
}

std::vector<std::string> DataProcessor::filterColumns(const ProcessedDataSet& data_set, const std::vector<std::string>& columns) {
    std::vector<std::string> filtered_columns;
    
    for (const std::string& col : columns) {
        if (std::find(data_set.columns.begin(), data_set.columns.end(), col) != data_set.columns.end()) {
            filtered_columns.push_back(col);
        }
    }
    
    return filtered_columns;
}

std::string DataProcessor::formatValue(const std::any& value, const std::string& format_type) const {
    std::string str_value = utils::anyToString(value);
    
    if (format_type == "number" && utils::isNumeric(str_value)) {
        double num_value = utils::toDouble(str_value);
        return utils::formatNumber(num_value, 2);
    } else if (format_type == "integer" && utils::isNumeric(str_value)) {
        int int_value = utils::toInt(str_value);
        return utils::formatInteger(int_value);
    } else if (format_type == "uppercase") {
        return utils::toUpper(str_value);
    } else if (format_type == "lowercase") {
        return utils::toLower(str_value);
    }
    
    return str_value;
}

ProcessedDataSet DataProcessor::sortDataSet(const ProcessedDataSet& data_set, const std::string& sort_column, bool ascending) {
    ProcessedDataSet sorted_data = data_set;
    
    if (std::find(data_set.columns.begin(), data_set.columns.end(), sort_column) == data_set.columns.end()) {
        return sorted_data; // Column not found, return original
    }
    
    std::sort(sorted_data.rows.begin(), sorted_data.rows.end(), 
        [&sort_column, ascending](const ProcessedRow& a, const ProcessedRow& b) {
            auto it_a = a.find(sort_column);
            auto it_b = b.find(sort_column);
            
            if (it_a == a.end() || it_b == b.end()) {
                return false;
            }
            
            std::string val_a = utils::anyToString(it_a->second);
            std::string val_b = utils::anyToString(it_b->second);
            
            // Try numeric comparison first
            if (utils::isNumeric(val_a) && utils::isNumeric(val_b)) {
                double num_a = utils::toDouble(val_a);
                double num_b = utils::toDouble(val_b);
                return ascending ? (num_a < num_b) : (num_a > num_b);
            }
            
            // Fall back to string comparison
            return ascending ? (val_a < val_b) : (val_a > val_b);
        });
    
    return sorted_data;
}

ProcessedDataSet DataProcessor::filterDataSet(const ProcessedDataSet& data_set, const std::string& filter_column, const std::string& filter_value) {
    ProcessedDataSet filtered_data = data_set;
    filtered_data.rows.clear();
    
    if (std::find(data_set.columns.begin(), data_set.columns.end(), filter_column) == data_set.columns.end()) {
        return filtered_data; // Column not found, return empty
    }
    
    for (const auto& row : data_set.rows) {
        auto it = row.find(filter_column);
        if (it != row.end()) {
            std::string value = utils::anyToString(it->second);
            if (utils::toLower(value).find(utils::toLower(filter_value)) != std::string::npos) {
                filtered_data.rows.push_back(row);
            }
        }
    }
    
    // Recalculate statistics for filtered data
    calculateStatistics(filtered_data);
    
    return filtered_data;
}

ProcessedDataSet DataProcessor::limitDataSet(const ProcessedDataSet& data_set, size_t max_rows) {
    ProcessedDataSet limited_data = data_set;
    
    if (limited_data.rows.size() > max_rows) {
        limited_data.rows.resize(max_rows);
        // Recalculate statistics for limited data
        calculateStatistics(limited_data);
    }
    
    return limited_data;
}

std::vector<std::pair<std::string, double>> DataProcessor::getNumericColumnData(const ProcessedDataSet& data_set, const std::string& column) {
    std::vector<std::pair<std::string, double>> numeric_data;
    
    if (std::find(data_set.columns.begin(), data_set.columns.end(), column) == data_set.columns.end()) {
        return numeric_data; // Column not found
    }
    
    for (const auto& row : data_set.rows) {
        auto it = row.find(column);
        if (it != row.end()) {
            std::string str_value = utils::anyToString(it->second);
            if (utils::isNumeric(str_value)) {
                double num_value = utils::toDouble(str_value);
                
                // Try to find a label column (first non-numeric column)
                std::string label = "Row " + std::to_string(numeric_data.size() + 1);
                for (const std::string& col : data_set.columns) {
                    if (col != column) {
                        auto label_it = row.find(col);
                        if (label_it != row.end()) {
                            std::string label_value = utils::anyToString(label_it->second);
                            if (!utils::isNumeric(label_value)) {
                                label = label_value;
                                break;
                            }
                        }
                    }
                }
                
                numeric_data.push_back({label, num_value});
            }
        }
    }
    
    return numeric_data;
}

bool DataProcessor::isColumnNumeric(const ProcessedDataSet& data_set, const std::string& column) {
    auto it = data_set.column_stats.find(column);
    if (it != data_set.column_stats.end()) {
        return it->second.is_numeric;
    }
    
    return false;
}

ColumnStatistics DataProcessor::getColumnStatistics(const ProcessedDataSet& data_set, const std::string& column) {
    auto it = data_set.column_stats.find(column);
    if (it != data_set.column_stats.end()) {
        return it->second;
    }
    
    // Return empty statistics if column not found
    return ColumnStatistics{};
}
