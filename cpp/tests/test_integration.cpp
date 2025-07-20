#include <iostream>
#include <cassert>
#include <filesystem>
#include "../include/vsr_app.h"
#include "../include/data_loader.h"
#include "../include/data_processor.h"
#include "../include/config_manager.h"
#include "../include/display_manager.h"
#include "../include/input_handler.h"
#include "../include/utils.h"

class TestIntegration {
private:
    std::string examples_dir_;
    
public:
    TestIntegration() : examples_dir_("../../examples") {
        // Check if examples directory exists
        if (!utils::directoryExists(examples_dir_)) {
            examples_dir_ = "../examples";
            if (!utils::directoryExists(examples_dir_)) {
                throw std::runtime_error("Examples directory not found");
            }
        }
    }
    
    void testCSVExample() {
        std::cout << "Testing CSV example integration..." << std::endl;
        
        std::string csv_file = examples_dir_ + "/sample_data.csv";
        if (!utils::fileExists(csv_file)) {
            std::cout << "⚠ CSV example file not found, skipping test" << std::endl;
            return;
        }
        
        DataLoader loader;
        bool loaded = loader.loadFromFile(csv_file);
        assert(loaded == true);
        
        auto data_sets = loader.getDataSets();
        assert(data_sets.size() >= 1);
        
        // Test data processing
        DataProcessor processor;
        processor.setDataSets(data_sets);
        
        DataProcessor::Preferences prefs;
        prefs.view_type = "table";
        prefs.slide_number = 1;
        prefs.selected_columns = loader.getColumnNames("main");
        
        processor.setPreferences("main", prefs);
        auto processed_data = processor.getProcessedData("main");
        
        assert(processed_data.size() > 0);
        
        std::cout << "✓ CSV example integration test passed" << std::endl;
    }
    
    void testFlatJSONExample() {
        std::cout << "Testing flat JSON example integration..." << std::endl;
        
        std::string json_file = examples_dir_ + "/flat_data.json";
        if (!utils::fileExists(json_file)) {
            std::cout << "⚠ Flat JSON example file not found, skipping test" << std::endl;
            return;
        }
        
        DataLoader loader;
        bool loaded = loader.loadFromFile(json_file);
        assert(loaded == true);
        
        auto data_sets = loader.getDataSets();
        assert(data_sets.size() >= 1);
        
        // Test data processing
        DataProcessor processor;
        processor.setDataSets(data_sets);
        
        for (const auto& [name, dataset] : data_sets) {
            DataProcessor::Preferences prefs;
            prefs.view_type = "table";
            prefs.slide_number = 1;
            prefs.selected_columns = loader.getColumnNames(name);
            
            processor.setPreferences(name, prefs);
            auto processed_data = processor.getProcessedData(name);
            
            assert(processed_data.size() > 0);
        }
        
        std::cout << "✓ Flat JSON example integration test passed" << std::endl;
    }
    
    void testComplexJSONExample() {
        std::cout << "Testing complex JSON example integration..." << std::endl;
        
        std::string json_file = examples_dir_ + "/complex_data.json";
        if (!utils::fileExists(json_file)) {
            std::cout << "⚠ Complex JSON example file not found, skipping test" << std::endl;
            return;
        }
        
        DataLoader loader;
        bool loaded = loader.loadFromFile(json_file);
        assert(loaded == true);
        
        auto data_sets = loader.getDataSets();
        assert(data_sets.size() >= 1);
        
        // Test data processing for all data sets
        DataProcessor processor;
        processor.setDataSets(data_sets);
        
        for (const auto& [name, dataset] : data_sets) {
            DataProcessor::Preferences prefs;
            prefs.view_type = "mixed";
            prefs.slide_number = 1;
            prefs.selected_columns = loader.getColumnNames(name);
            
            processor.setPreferences(name, prefs);
            auto processed_data = processor.getProcessedData(name);
            
            // Should have some data
            assert(processed_data.size() > 0);
        }
        
        std::cout << "✓ Complex JSON example integration test passed" << std::endl;
    }
    
    void testMultipleSetsExample() {
        std::cout << "Testing multiple sets JSON example integration..." << std::endl;
        
        std::string json_file = examples_dir_ + "/multiple_sets_data.json";
        if (!utils::fileExists(json_file)) {
            std::cout << "⚠ Multiple sets JSON example file not found, skipping test" << std::endl;
            return;
        }
        
        DataLoader loader;
        bool loaded = loader.loadFromFile(json_file);
        assert(loaded == true);
        
        auto data_sets = loader.getDataSets();
        assert(data_sets.size() > 1); // Should have multiple data sets
        
        // Test slide organization
        DataProcessor processor;
        processor.setDataSets(data_sets);
        
        int slide_num = 1;
        for (const auto& [name, dataset] : data_sets) {
            DataProcessor::Preferences prefs;
            prefs.view_type = "table";
            prefs.slide_number = slide_num++;
            prefs.selected_columns = loader.getColumnNames(name);
            
            processor.setPreferences(name, prefs);
            auto processed_data = processor.getProcessedData(name);
            
            assert(processed_data.size() > 0);
        }
        
        std::cout << "✓ Multiple sets JSON example integration test passed" << std::endl;
    }
    
    void testNestedTreeExample() {
        std::cout << "Testing nested tree JSON example integration..." << std::endl;
        
        std::string json_file = examples_dir_ + "/nested_tree_data.json";
        if (!utils::fileExists(json_file)) {
            std::cout << "⚠ Nested tree JSON example file not found, skipping test" << std::endl;
            return;
        }
        
        DataLoader loader;
        bool loaded = loader.loadFromFile(json_file);
        assert(loaded == true);
        
        auto data_sets = loader.getDataSets();
        assert(data_sets.size() >= 1);
        
        // Test tree view processing
        DataProcessor processor;
        processor.setDataSets(data_sets);
        
        for (const auto& [name, dataset] : data_sets) {
            DataProcessor::Preferences prefs;
            prefs.view_type = "tree";
            prefs.slide_number = 1;
            prefs.selected_columns = loader.getColumnNames(name);
            
            processor.setPreferences(name, prefs);
            auto processed_data = processor.getProcessedData(name);
            
            assert(processed_data.size() > 0);
        }
        
        std::cout << "✓ Nested tree JSON example integration test passed" << std::endl;
    }
    
    void testConfigurationPersistence() {
        std::cout << "Testing configuration persistence..." << std::endl;
        
        std::string test_file = examples_dir_ + "/sample_data.csv";
        if (!utils::fileExists(test_file)) {
            std::cout << "⚠ Test file not found, skipping configuration test" << std::endl;
            return;
        }
        
        ConfigManager config_manager;
        
        // Create test preferences
        DataProcessor::Preferences prefs;
        prefs.view_type = "bars";
        prefs.slide_number = 2;
        prefs.selected_columns = {"name", "population"};
        
        // Save configuration
        config_manager.saveConfig(test_file, "main", prefs);
        
        // Load configuration
        auto loaded_prefs = config_manager.loadConfig(test_file, "main");
        assert(loaded_prefs.has_value());
        
        // Verify loaded preferences
        assert(loaded_prefs->view_type == "bars");
        assert(loaded_prefs->slide_number == 2);
        assert(loaded_prefs->selected_columns.size() == 2);
        assert(loaded_prefs->selected_columns[0] == "name");
        assert(loaded_prefs->selected_columns[1] == "population");
        
        // Clean up
        config_manager.deleteConfig(test_file, "main");
        
        std::cout << "✓ Configuration persistence test passed" << std::endl;
    }
    
    void testFullApplicationWorkflow() {
        std::cout << "Testing full application workflow..." << std::endl;
        
        std::string test_file = examples_dir_ + "/sample_data.csv";
        if (!utils::fileExists(test_file)) {
            std::cout << "⚠ Test file not found, skipping workflow test" << std::endl;
            return;
        }
        
        try {
            // Test VSRApp initialization
            VSRApp app;
            app.initialize();
            
            // Test data loading
            bool loaded = app.loadData(test_file);
            assert(loaded == true);
            
            // Test data processing
            app.processData();
            
            // Test display update (should not throw)
            app.updateDisplay();
            
            // Test shutdown
            app.shutdown();
            
            std::cout << "✓ Full application workflow test passed" << std::endl;
            
        } catch (const std::exception& e) {
            std::cout << "Full workflow test failed: " << e.what() << std::endl;
            throw;
        }
    }
    
    void testLargeDataHandling() {
        std::cout << "Testing large data handling..." << std::endl;
        
        std::string large_file = examples_dir_ + "/large_user_data.json";
        if (!utils::fileExists(large_file)) {
            std::cout << "⚠ Large data file not found, skipping test" << std::endl;
            return;
        }
        
        DataLoader loader;
        bool loaded = loader.loadFromFile(large_file);
        assert(loaded == true);
        
        auto data_sets = loader.getDataSets();
        assert(data_sets.size() >= 1);
        
        // Test processing large data
        DataProcessor processor;
        processor.setDataSets(data_sets);
        
        for (const auto& [name, dataset] : data_sets) {
            DataProcessor::Preferences prefs;
            prefs.view_type = "table";
            prefs.slide_number = 1;
            prefs.selected_columns = loader.getColumnNames(name);
            
            processor.setPreferences(name, prefs);
            auto processed_data = processor.getProcessedData(name);
            
            // Should handle large data without issues
            assert(processed_data.size() > 0);
        }
        
        std::cout << "✓ Large data handling test passed" << std::endl;
    }
    
    void runAllTests() {
        std::cout << "=== Integration Tests ===" << std::endl;
        std::cout << "Using examples directory: " << examples_dir_ << std::endl;
        
        try {
            testCSVExample();
            testFlatJSONExample();
            testComplexJSONExample();
            testMultipleSetsExample();
            testNestedTreeExample();
            testConfigurationPersistence();
            testFullApplicationWorkflow();
            testLargeDataHandling();
            
            std::cout << "All Integration tests passed!" << std::endl;
            
        } catch (const std::exception& e) {
            std::cout << "Integration test failed: " << e.what() << std::endl;
            throw;
        }
    }
};

int main() {
    try {
        utils::enableUTF8Console();
        
        TestIntegration test;
        test.runAllTests();
        
        std::cout << "\nPress any key to exit..." << std::endl;
        std::cin.get();
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "Integration test suite failed: " << e.what() << std::endl;
        std::cin.get();
        return 1;
    }
}
