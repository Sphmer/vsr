#pragma once

#include <string>
#include <vector>
#include <map>
#include <any>
#include "data_loader.h"

class DataProcessor {
public:
    DataProcessor() = default;
    ~DataProcessor() = default;

    // Main processing methods
    std::vector<ProcessedDataSet> processDataSets(
        const std::map<std::string, DataSet>& data_sets,
        const std::map<std::string, DataSetPreference>& preferences
    );

    ProcessedDataSet processDataSet(
        const DataSet& data_set,
        const DataSetPreference& preference
    );

    // Data transformation methods
    std::vector<std::map<std::string, std::string>> convertToStringMaps(
        const std::vector<std::map<std::string, std::any>>& data
    );

    std::vector<std::string> extractColumns(
        const std::vector<std::map<std::string, std::any>>& data,
        bool preserve_order = true
    );

    std::vector<std::string> filterSelectedColumns(
        const std::vector<std::string>& all_columns,
        const std::vector<std::string>& selected_columns,
        const std::vector<std::string>& manual_order = {}
    );

    // Statistics and analysis methods
    void calculateStatistics(ProcessedDataSet& processed);
    std::vector<std::string> convertDataToStrings(const ProcessedDataSet& data_set);
    std::vector<std::string> filterColumns(const ProcessedDataSet& data_set, const std::vector<std::string>& columns);
    
    // Data manipulation methods
    ProcessedDataSet sortDataSet(const ProcessedDataSet& data_set, const std::string& sort_column, bool ascending = true);
    ProcessedDataSet filterDataSet(const ProcessedDataSet& data_set, const std::string& filter_column, const std::string& filter_value);
    ProcessedDataSet limitDataSet(const ProcessedDataSet& data_set, size_t max_rows);
    
    // Column analysis methods
    std::vector<std::pair<std::string, double>> getNumericColumnData(const ProcessedDataSet& data_set, const std::string& column);
    bool isColumnNumeric(const ProcessedDataSet& data_set, const std::string& column);
    ColumnStatistics getColumnStatistics(const ProcessedDataSet& data_set, const std::string& column);
    
    // Utility methods
    std::string formatValue(const std::any& value, const std::string& format_type = "default") const;
    bool isNumericValue(const std::any& value) const;
    double getNumericValue(const std::any& value) const;
    std::string truncateString(const std::string& str, size_t max_length) const;

private:
    // Helper methods
    std::vector<std::map<std::string, std::string>> processTableData(
        const DataSet& data_set,
        const std::vector<std::string>& selected_columns
    );

    std::vector<std::map<std::string, std::string>> processBarData(
        const DataSet& data_set,
        const std::string& bar_field
    );

    std::vector<std::map<std::string, std::string>> processTreeData(
        const DataSet& data_set
    );
};
