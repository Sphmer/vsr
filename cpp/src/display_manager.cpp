#include "display_manager.h"
#include "utils.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>

DisplayManager::DisplayManager() {
    auto console_size = utils::getConsoleSize();
    terminal_width_ = console_size.first;
    terminal_height_ = console_size.second;
    
    utils::log(utils::LogLevel::DEBUG, "DisplayManager initialized with terminal size: " + 
              std::to_string(terminal_width_) + "x" + std::to_string(terminal_height_));
}

void DisplayManager::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    
    // Alternative method using ANSI escape codes
    std::cout << "\033[2J\033[H" << std::flush;
}

void DisplayManager::displayTableView(const std::vector<ProcessedDataSet>& data_sets, int scroll_offset, int max_rows) {
    if (data_sets.empty()) {
        std::cout << "No data to display." << std::endl;
        return;
    }
    
    for (const auto& data_set : data_sets) {
        displayTableForDataSet(data_set, scroll_offset, max_rows);
        std::cout << std::endl;
    }
}

void DisplayManager::displayBarView(const std::vector<ProcessedDataSet>& data_sets, int scroll_offset, int max_rows) {
    if (data_sets.empty()) {
        std::cout << "No data to display." << std::endl;
        return;
    }
    
    for (const auto& data_set : data_sets) {
        displayBarChartForDataSet(data_set, scroll_offset, max_rows);
        std::cout << std::endl;
    }
}

void DisplayManager::displayTreeView(const std::vector<ProcessedDataSet>& data_sets, int scroll_offset, int max_rows) {
    if (data_sets.empty()) {
        std::cout << "No data to display." << std::endl;
        return;
    }
    
    for (const auto& data_set : data_sets) {
        displayTreeForDataSet(data_set, scroll_offset, max_rows);
        std::cout << std::endl;
    }
}

void DisplayManager::displayMixedView(const std::vector<ProcessedDataSet>& data_sets, int scroll_offset, int max_rows) {
    if (data_sets.empty()) {
        std::cout << "No data to display." << std::endl;
        return;
    }
    
    for (const auto& data_set : data_sets) {
        std::cout << "=== " << data_set.set_name << " ===" << std::endl;
        
        if (data_set.view_type == "table") {
            displayTableForDataSet(data_set, scroll_offset, max_rows);
        } else if (data_set.view_type == "bars") {
            displayBarChartForDataSet(data_set, scroll_offset, max_rows);
        } else if (data_set.view_type == "tree") {
            displayTreeForDataSet(data_set, scroll_offset, max_rows);
        } else {
            // Default to table view
            displayTableForDataSet(data_set, scroll_offset, max_rows);
        }
        
        std::cout << std::endl;
    }
}

void DisplayManager::displayTableForDataSet(const ProcessedDataSet& data_set, int scroll_offset, int max_rows) {
    if (data_set.rows.empty()) {
        std::cout << "No data in set: " << data_set.set_name << std::endl;
        return;
    }
    
    // Calculate column widths
    std::vector<int> column_widths;
    for (const std::string& col : data_set.columns) {
        int max_width = col.length();
        
        for (const auto& row : data_set.rows) {
            auto it = row.find(col);
            if (it != row.end()) {
                std::string value = utils::anyToString(it->second);
                max_width = std::max(max_width, static_cast<int>(value.length()));
            }
        }
        
        // Limit column width to reasonable size
        max_width = std::min(max_width, 30);
        column_widths.push_back(max_width);
    }
    
    // Display header
    displayTableHeader(data_set.columns, column_widths);
    displayTableSeparator(column_widths);
    
    // Display data rows with scrolling
    int displayed_rows = 0;
    int current_row = 0;
    
    for (const auto& row : data_set.rows) {
        if (current_row >= scroll_offset && displayed_rows < max_rows) {
            displayTableRow(row, data_set.columns, column_widths);
            displayed_rows++;
        }
        current_row++;
    }
    
    // Display scroll indicator
    if (scroll_offset > 0 || current_row > scroll_offset + max_rows) {
        std::cout << "Showing rows " << (scroll_offset + 1) << "-" << (scroll_offset + displayed_rows) 
                  << " of " << data_set.rows.size() << std::endl;
    }
}

void DisplayManager::displayBarChartForDataSet(const ProcessedDataSet& data_set, int scroll_offset, int max_rows) {
    if (data_set.rows.empty() || data_set.columns.empty()) {
        std::cout << "No data for bar chart: " << data_set.set_name << std::endl;
        return;
    }
    
    // Find the first numeric column for the bar chart
    std::string numeric_column;
    std::string label_column;
    
    for (const std::string& col : data_set.columns) {
        auto stats_it = data_set.column_stats.find(col);
        if (stats_it != data_set.column_stats.end() && stats_it->second.is_numeric) {
            numeric_column = col;
            break;
        }
    }
    
    if (numeric_column.empty()) {
        std::cout << "No numeric column found for bar chart: " << data_set.set_name << std::endl;
        return;
    }
    
    // Find a label column (first non-numeric column)
    for (const std::string& col : data_set.columns) {
        if (col != numeric_column) {
            auto stats_it = data_set.column_stats.find(col);
            if (stats_it == data_set.column_stats.end() || !stats_it->second.is_numeric) {
                label_column = col;
                break;
            }
        }
    }
    
    if (label_column.empty()) {
        label_column = "Row";
    }
    
    std::cout << "Bar Chart: " << numeric_column << " by " << label_column << std::endl;
    
    // Get numeric data with scrolling
    std::vector<std::pair<std::string, double>> chart_data;
    int current_row = 0;
    int displayed_rows = 0;
    
    for (const auto& row : data_set.rows) {
        if (current_row >= scroll_offset && displayed_rows < max_rows) {
            auto numeric_it = row.find(numeric_column);
            auto label_it = row.find(label_column);
            
            if (numeric_it != row.end()) {
                std::string numeric_str = utils::anyToString(numeric_it->second);
                if (utils::isNumeric(numeric_str)) {
                    double value = utils::toDouble(numeric_str);
                    std::string label = (label_it != row.end()) ? 
                                      utils::anyToString(label_it->second) : 
                                      ("Row " + std::to_string(current_row + 1));
                    
                    chart_data.push_back({label, value});
                    displayed_rows++;
                }
            }
        }
        current_row++;
    }
    
    if (chart_data.empty()) {
        std::cout << "No numeric data to display." << std::endl;
        return;
    }
    
    // Find max value for scaling
    double max_value = 0;
    for (const auto& [label, value] : chart_data) {
        max_value = std::max(max_value, std::abs(value));
    }
    
    if (max_value == 0) {
        std::cout << "All values are zero." << std::endl;
        return;
    }
    
    // Display bars
    int bar_width = std::min(50, terminal_width_ - 30); // Leave space for labels and values
    
    for (const auto& [label, value] : chart_data) {
        int bar_length = static_cast<int>((std::abs(value) / max_value) * bar_width);
        
        std::cout << std::setw(15) << std::left << label.substr(0, 14) << " ";
        std::cout << std::setw(8) << std::right << utils::formatNumber(value, 2) << " ";
        
        // Draw bar
        std::string bar(bar_length, '#');
        std::cout << bar << std::endl;
    }
}

void DisplayManager::displayTreeForDataSet(const ProcessedDataSet& data_set, int scroll_offset, int max_rows) {
    if (data_set.rows.empty()) {
        std::cout << "No data for tree view: " << data_set.set_name << std::endl;
        return;
    }
    
    std::cout << "Tree View: " << data_set.set_name << std::endl;
    std::cout << "├── Columns: " << data_set.columns.size() << std::endl;
    std::cout << "├── Rows: " << data_set.rows.size() << std::endl;
    
    // Display column information
    for (size_t i = 0; i < data_set.columns.size(); ++i) {
        const std::string& col = data_set.columns[i];
        bool is_last_column = (i == data_set.columns.size() - 1);
        
        std::cout << (is_last_column ? "└── " : "├── ") << col;
        
        // Add column statistics
        auto stats_it = data_set.column_stats.find(col);
        if (stats_it != data_set.column_stats.end()) {
            const auto& stats = stats_it->second;
            if (stats.is_numeric) {
                std::cout << " (numeric: " << utils::formatNumber(stats.min_value, 2) 
                          << " - " << utils::formatNumber(stats.max_value, 2) << ")";
            } else {
                std::cout << " (text)";
            }
        }
        
        std::cout << std::endl;
        
        // Display sample data with scrolling
        if (!is_last_column) {
            int sample_count = 0;
            int current_row = 0;
            
            for (const auto& row : data_set.rows) {
                if (current_row >= scroll_offset && sample_count < 3) {
                    auto it = row.find(col);
                    if (it != row.end()) {
                        std::string value = utils::anyToString(it->second);
                        if (value.length() > 20) {
                            value = value.substr(0, 17) + "...";
                        }
                        std::cout << "│   └── " << value << std::endl;
                        sample_count++;
                    }
                }
                current_row++;
            }
        }
    }
}

void DisplayManager::displaySlideInfo(int current_slide, int total_slides) {
    if (total_slides > 1) {
        std::cout << "\nSlide " << current_slide << " of " << total_slides << std::endl;
    }
}

void DisplayManager::displayHelp() {
    std::cout << "=== VSR Help ===" << std::endl;
    std::cout << std::endl;
    std::cout << "Navigation:" << std::endl;
    std::cout << "  ↑/k       - Scroll up" << std::endl;
    std::cout << "  ↓/j       - Scroll down" << std::endl;
    std::cout << "  ←/h       - Previous slide" << std::endl;
    std::cout << "  →/l       - Next slide" << std::endl;
    std::cout << "  PageUp    - Scroll up one page" << std::endl;
    std::cout << "  PageDown  - Scroll down one page" << std::endl;
    std::cout << "  Home      - Go to top" << std::endl;
    std::cout << std::endl;
    std::cout << "View Modes:" << std::endl;
    std::cout << "  t         - Table view" << std::endl;
    std::cout << "  b         - Bar chart view" << std::endl;
    std::cout << "  m         - Mixed view (default)" << std::endl;
    std::cout << std::endl;
    std::cout << "Configuration:" << std::endl;
    std::cout << "  r         - Reconfigure representations" << std::endl;
    std::cout << std::endl;
    std::cout << "Other:" << std::endl;
    std::cout << "  h         - Show this help" << std::endl;
    std::cout << "  q         - Quit application" << std::endl;
}

std::string DisplayManager::showFileSelectionMenu(const std::vector<std::string>& files) {
    if (files.empty()) {
        std::cout << "No files available." << std::endl;
        return "";
    }
    
    if (files.size() == 1) {
        return files[0];
    }
    
    std::cout << "Select a file:" << std::endl;
    for (size_t i = 0; i < files.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << files[i] << std::endl;
    }
    
    std::cout << "Enter choice (1-" << files.size() << "): ";
    
    std::string input;
    std::getline(std::cin, input);
    
    int choice = utils::toInt(utils::trim(input));
    if (choice >= 1 && choice <= static_cast<int>(files.size())) {
        return files[choice - 1];
    }
    
    return files[0]; // Default to first file
}

void DisplayManager::displayTableHeader(const std::vector<std::string>& columns, const std::vector<int>& column_widths) {
    std::cout << "│";
    for (size_t i = 0; i < columns.size(); ++i) {
        std::cout << " " << std::setw(column_widths[i]) << std::left << columns[i] << " │";
    }
    std::cout << std::endl;
}

void DisplayManager::displayTableSeparator(const std::vector<int>& column_widths) {
    std::cout << "├";
    for (size_t i = 0; i < column_widths.size(); ++i) {
        std::cout << std::string(column_widths[i] + 2, '-');
        if (i < column_widths.size() - 1) {
            std::cout << "┼";
        }
    }
    std::cout << "┤" << std::endl;
}

void DisplayManager::displayTableRow(const ProcessedRow& row, const std::vector<std::string>& columns, const std::vector<int>& column_widths) {
    std::cout << "│";
    for (size_t i = 0; i < columns.size(); ++i) {
        std::string value = "N/A";
        auto it = row.find(columns[i]);
        if (it != row.end()) {
            value = utils::anyToString(it->second);
            if (value.length() > static_cast<size_t>(column_widths[i])) {
                value = value.substr(0, column_widths[i] - 3) + "...";
            }
        }
        
        std::cout << " " << std::setw(column_widths[i]) << std::left << value << " │";
    }
    std::cout << std::endl;
}

void DisplayManager::updateTerminalSize() {
    auto console_size = utils::getConsoleSize();
    terminal_width_ = console_size.first;
    terminal_height_ = console_size.second;
}

void DisplayManager::displayStatus(const std::string& message) {
    std::cout << "[STATUS] " << message << std::endl;
}

void DisplayManager::displayError(const std::string& error_message) {
    std::cout << "[ERROR] " << error_message << std::endl;
}

void DisplayManager::displayWarning(const std::string& warning_message) {
    std::cout << "[WARNING] " << warning_message << std::endl;
}
