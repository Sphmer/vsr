#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include "../include/display_manager.h"
#include "../include/data_loader.h"
#include "../include/data_processor.h"
#include "../include/utils.h"

class TestDisplay {
private:
    DisplayManager display_;
    
public:
    void createSampleData(DataLoader& loader) {
        // Create sample data for testing display
        std::string csv_content = 
            "name,population,state\n"
            "New York,8419000,NY\n"
            "Los Angeles,3980000,CA\n"
            "Chicago,2716000,IL\n"
            "Houston,2320000,TX\n"
            "Phoenix,1680000,AZ\n";
        
        utils::writeFile("test_display.csv", csv_content);
        loader.loadFromFile("test_display.csv");
    }
    
    void testTableDisplay() {
        std::cout << "Testing table display..." << std::endl;
        
        DataLoader loader;
        createSampleData(loader);
        
        DataProcessor processor;
        processor.setDataSets(loader.getDataSets());
        
        // Configure for table view
        DataProcessor::Preferences prefs;
        prefs.view_type = "table";
        prefs.slide_number = 1;
        prefs.selected_columns = {"name", "population", "state"};
        
        processor.setPreferences("main", prefs);
        auto processed_data = processor.getProcessedData("main");
        
        // Test table rendering (should not throw exceptions)
        try {
            display_.renderTable(processed_data, {"name", "population", "state"}, 0, 80, 20);
            std::cout << "✓ Table display test passed" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Table display failed: " << e.what() << std::endl;
            throw;
        }
    }
    
    void testBarDisplay() {
        std::cout << "Testing bar display..." << std::endl;
        
        DataLoader loader;
        createSampleData(loader);
        
        DataProcessor processor;
        processor.setDataSets(loader.getDataSets());
        
        // Configure for bar view
        DataProcessor::Preferences prefs;
        prefs.view_type = "bars";
        prefs.slide_number = 1;
        prefs.selected_columns = {"name", "population"};
        
        processor.setPreferences("main", prefs);
        auto processed_data = processor.getProcessedData("main");
        
        // Test bar rendering (should not throw exceptions)
        try {
            display_.renderBars(processed_data, {"name", "population"}, 0, 80, 20);
            std::cout << "✓ Bar display test passed" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Bar display failed: " << e.what() << std::endl;
            throw;
        }
    }
    
    void testTreeDisplay() {
        std::cout << "Testing tree display..." << std::endl;
        
        // Create nested JSON for tree display
        std::string json_content = 
            "{\n"
            "  \"users\": [\n"
            "    {\"name\": \"John\", \"age\": 30, \"department\": \"Engineering\"},\n"
            "    {\"name\": \"Jane\", \"age\": 25, \"department\": \"Marketing\"}\n"
            "  ],\n"
            "  \"departments\": [\n"
            "    {\"name\": \"Engineering\", \"budget\": 100000},\n"
            "    {\"name\": \"Marketing\", \"budget\": 50000}\n"
            "  ]\n"
            "}";
        
        utils::writeFile("test_tree.json", json_content);
        
        DataLoader loader;
        loader.loadFromFile("test_tree.json");
        
        DataProcessor processor;
        processor.setDataSets(loader.getDataSets());
        
        // Configure for tree view
        DataProcessor::Preferences prefs;
        prefs.view_type = "tree";
        prefs.slide_number = 1;
        prefs.selected_columns = {"name", "age", "department"};
        
        processor.setPreferences("users", prefs);
        auto processed_data = processor.getProcessedData("users");
        
        // Test tree rendering (should not throw exceptions)
        try {
            display_.renderTree(processed_data, {"name", "age", "department"}, 0, 80, 20);
            std::cout << "✓ Tree display test passed" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Tree display failed: " << e.what() << std::endl;
            throw;
        }
    }
    
    void testMixedDisplay() {
        std::cout << "Testing mixed display..." << std::endl;
        
        DataLoader loader;
        createSampleData(loader);
        
        DataProcessor processor;
        processor.setDataSets(loader.getDataSets());
        
        // Configure for mixed view
        DataProcessor::Preferences prefs;
        prefs.view_type = "mixed";
        prefs.slide_number = 1;
        prefs.selected_columns = {"name", "population", "state"};
        
        processor.setPreferences("main", prefs);
        auto processed_data = processor.getProcessedData("main");
        
        // Test mixed rendering (should not throw exceptions)
        try {
            display_.renderMixed(processed_data, {"name", "population", "state"}, 0, 80, 20);
            std::cout << "✓ Mixed display test passed" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Mixed display failed: " << e.what() << std::endl;
            throw;
        }
    }
    
    void testScreenOperations() {
        std::cout << "Testing screen operations..." << std::endl;
        
        try {
            // Test screen clearing (should not throw exceptions)
            display_.clearScreen();
            
            // Test slide info display
            display_.showSlideInfo(1, 3, "Test Data");
            
            // Test help display
            display_.showHelp();
            
            // Test status messages
            display_.showStatus("Test status message");
            display_.showError("Test error message");
            display_.showWarning("Test warning message");
            
            std::cout << "✓ Screen operations test passed" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Screen operations failed: " << e.what() << std::endl;
            throw;
        }
    }
    
    void testFileSelection() {
        std::cout << "Testing file selection menu..." << std::endl;
        
        try {
            std::vector<std::string> files = {
                "sample_data.csv",
                "complex_data.json",
                "flat_data.json"
            };
            
            // Test file selection display (should not throw exceptions)
            display_.showFileSelection(files);
            
            std::cout << "✓ File selection test passed" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "File selection failed: " << e.what() << std::endl;
            throw;
        }
    }
    
    void testColumnWidthCalculation() {
        std::cout << "Testing column width calculation..." << std::endl;
        
        DataLoader loader;
        createSampleData(loader);
        
        DataProcessor processor;
        processor.setDataSets(loader.getDataSets());
        
        DataProcessor::Preferences prefs;
        prefs.view_type = "table";
        prefs.slide_number = 1;
        prefs.selected_columns = {"name", "population", "state"};
        
        processor.setPreferences("main", prefs);
        auto processed_data = processor.getProcessedData("main");
        
        try {
            // Test column width calculation
            auto widths = display_.calculateColumnWidths(processed_data, {"name", "population", "state"}, 80);
            
            assert(widths.size() == 3);
            assert(widths[0] > 0);  // Name column width
            assert(widths[1] > 0);  // Population column width
            assert(widths[2] > 0);  // State column width
            
            // Total width should not exceed available width
            int total_width = 0;
            for (auto width : widths) {
                total_width += width;
            }
            assert(total_width <= 80);
            
            std::cout << "✓ Column width calculation test passed" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Column width calculation failed: " << e.what() << std::endl;
            throw;
        }
    }
    
    void cleanup() {
        // Clean up test files
        try {
            std::filesystem::remove("test_display.csv");
            std::filesystem::remove("test_tree.json");
        } catch (...) {
            // Ignore cleanup errors
        }
    }
    
    void runAllTests() {
        std::cout << "=== Display Tests ===" << std::endl;
        
        try {
            testTableDisplay();
            testBarDisplay();
            testTreeDisplay();
            testMixedDisplay();
            testScreenOperations();
            testFileSelection();
            testColumnWidthCalculation();
            
            cleanup();
            
            std::cout << "All Display tests passed!" << std::endl;
            
        } catch (const std::exception& e) {
            cleanup();
            std::cout << "Test failed: " << e.what() << std::endl;
            throw;
        }
    }
};

int main() {
    try {
        utils::enableUTF8Console();
        
        TestDisplay test;
        test.runAllTests();
        
        std::cout << "\nPress any key to exit..." << std::endl;
        std::cin.get();
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "Test suite failed: " << e.what() << std::endl;
        std::cin.get();
        return 1;
    }
}
