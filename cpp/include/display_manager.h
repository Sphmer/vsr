#pragma once

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include "data_loader.h"

class DisplayManager {
public:
    DisplayManager();
    ~DisplayManager() = default;

    // Terminal management
    void clearScreen();
    void getTerminalSize(int& width, int& height);
    void setCursorPosition(int x, int y);
    void hideCursor();
    void showCursor();

    // Display methods
    void displayTableView(const std::vector<ProcessedData>& data, int scroll_offset, int max_rows);
    void displayBarView(const std::vector<ProcessedData>& data, int scroll_offset, int max_rows);
    void displayTreeView(const std::vector<ProcessedData>& data, int scroll_offset, int max_rows);
    void displayMixedView(const std::vector<ProcessedData>& data, int scroll_offset, int max_rows);
    void displayHelp();

    // Individual view creators
    std::vector<std::string> createTableViewForSet(
        const std::string& set_name,
        const std::vector<std::map<std::string, std::string>>& data,
        const std::vector<std::string>& columns,
        int max_lines
    );

    std::vector<std::string> createBarViewForSet(
        const std::string& set_name,
        const std::vector<std::map<std::string, std::string>>& data,
        const std::string& bar_field,
        int max_lines
    );

    std::vector<std::string> createTreeViewForSet(
        const std::string& set_name,
        const std::vector<std::map<std::string, std::string>>& data,
        int max_lines
    );

    // Status and navigation
    void displayStatus(const std::string& message);
    void displayError(const std::string& error_message);
    void displayWarning(const std::string& warning_message);
    
    // Helper methods for individual data set display
    void displayTableForDataSet(const ProcessedDataSet& data_set, int scroll_offset, int max_rows);
    void displayBarChartForDataSet(const ProcessedDataSet& data_set, int scroll_offset, int max_rows);
    void displayTreeForDataSet(const ProcessedDataSet& data_set, int scroll_offset, int max_rows);
    
    // Table formatting helpers
    void displayTableHeader(const std::vector<std::string>& columns, const std::vector<int>& column_widths);
    void displayTableSeparator(const std::vector<int>& column_widths);
    void displayTableRow(const ProcessedRow& row, const std::vector<std::string>& columns, const std::vector<int>& column_widths);
    
    // Terminal utilities
    void updateTerminalSize();
    void displaySlideInfo(int current_slide, int total_slides);
    void displayScrollInfo(int scroll_offset, int total_lines);

    // Interactive menus
    std::string showFileSelectionMenu(const std::vector<std::string>& files);
    int showSelectionMenu(
        const std::string& title,
        const std::vector<std::string>& options,
        int selected_index = 0
    );

    // Utility methods
    std::string formatTableRow(const std::vector<std::string>& cells, const std::vector<int>& widths);
    std::string createHorizontalLine(const std::vector<int>& widths);
    std::string createBarChart(const std::string& label, double value, double max_value, int bar_width);
    std::string truncateText(const std::string& text, int max_width);
    std::vector<int> calculateColumnWidths(
        const std::vector<std::map<std::string, std::string>>& data,
        const std::vector<std::string>& columns,
        int terminal_width
    );

private:
    int terminal_width_;
    int terminal_height_;
    
    // Helper methods
    void renderTreeNode(
        const std::map<std::string, std::string>& node,
        std::vector<std::string>& lines,
        const std::string& prefix,
        bool is_last,
        int max_lines,
        const std::string& key = ""
    );

    std::string escapeAnsiSequences(const std::string& text);
    void initializeTerminal();
    
    // Platform-specific methods
    void enableVirtualTerminalProcessing();
    void disableVirtualTerminalProcessing();
};
