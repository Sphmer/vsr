#include <iostream>
#include <cassert>
#include <fstream>
#include <filesystem>
#include "../include/data_loader.h"
#include "../include/utils.h"

class TestDataLoader {
private:
    std::string test_dir_;
    
public:
    TestDataLoader() : test_dir_("test_data") {
        // Create test directory
        utils::createDirectory(test_dir_);
    }
    
    ~TestDataLoader() {
        // Clean up test files
        try {
            std::filesystem::remove_all(test_dir_);
        } catch (...) {
            // Ignore cleanup errors
        }
    }
    
    void createTestCSV() {
        std::string csv_content = 
            "name,population,state\n"
            "New York,8419000,NY\n"
            "Los Angeles,3980000,CA\n"
            "Chicago,2716000,IL\n";
        
        utils::writeFile(test_dir_ + "/test.csv", csv_content);
    }
    
    void createTestJSON() {
        std::string json_content = 
            "[\n"
            "  {\"name\": \"John\", \"age\": 30, \"city\": \"New York\"},\n"
            "  {\"name\": \"Jane\", \"age\": 25, \"city\": \"Los Angeles\"},\n"
            "  {\"name\": \"Bob\", \"age\": 35, \"city\": \"Chicago\"}\n"
            "]";
        
        utils::writeFile(test_dir_ + "/test.json", json_content);
    }
    
    void createNestedJSON() {
        std::string json_content = 
            "{\n"
            "  \"users\": [\n"
            "    {\"name\": \"John\", \"age\": 30},\n"
            "    {\"name\": \"Jane\", \"age\": 25}\n"
            "  ],\n"
            "  \"products\": [\n"
            "    {\"name\": \"Laptop\", \"price\": 999.99},\n"
            "    {\"name\": \"Mouse\", \"price\": 29.99}\n"
            "  ]\n"
            "}";
        
        utils::writeFile(test_dir_ + "/nested.json", json_content);
    }
    
    void testCSVLoading() {
        std::cout << "Testing CSV loading..." << std::endl;
        
        createTestCSV();
        
        DataLoader loader;
        bool result = loader.loadFromFile(test_dir_ + "/test.csv");
        
        assert(result == true);
        
        auto data_sets = loader.getDataSets();
        assert(data_sets.size() == 1);
        assert(data_sets.find("main") != data_sets.end());
        
        const DataSet& main_set = data_sets["main"];
        assert(main_set.rows.size() == 3);
        assert(main_set.type == DataSetType::CSV);
        
        // Test column names
        auto columns = loader.getColumnNames("main");
        assert(columns.size() == 3);
        assert(columns[0] == "name");
        assert(columns[1] == "population");
        assert(columns[2] == "state");
        
        std::cout << "✓ CSV loading test passed" << std::endl;
    }
    
    void testJSONLoading() {
        std::cout << "Testing JSON loading..." << std::endl;
        
        createTestJSON();
        
        DataLoader loader;
        bool result = loader.loadFromFile(test_dir_ + "/test.json");
        
        assert(result == true);
        
        auto data_sets = loader.getDataSets();
        assert(data_sets.size() == 1);
        assert(data_sets.find("main") != data_sets.end());
        
        const DataSet& main_set = data_sets["main"];
        assert(main_set.rows.size() == 3);
        assert(main_set.type == DataSetType::ARRAY);
        
        // Test first row data
        const auto& first_row = main_set.rows[0];
        assert(first_row.find("name") != first_row.end());
        assert(utils::anyToString(first_row.at("name")) == "John");
        
        std::cout << "✓ JSON loading test passed" << std::endl;
    }
    
    void testNestedJSONLoading() {
        std::cout << "Testing nested JSON loading..." << std::endl;
        
        createNestedJSON();
        
        DataLoader loader;
        bool result = loader.loadFromFile(test_dir_ + "/nested.json");
        
        assert(result == true);
        
        auto data_sets = loader.getDataSets();
        assert(data_sets.size() == 2);
        assert(data_sets.find("users") != data_sets.end());
        assert(data_sets.find("products") != data_sets.end());
        
        const DataSet& users_set = data_sets["users"];
        assert(users_set.rows.size() == 2);
        assert(users_set.type == DataSetType::NESTED);
        
        const DataSet& products_set = data_sets["products"];
        assert(products_set.rows.size() == 2);
        assert(products_set.type == DataSetType::NESTED);
        
        std::cout << "✓ Nested JSON loading test passed" << std::endl;
    }
    
    void testInvalidFile() {
        std::cout << "Testing invalid file handling..." << std::endl;
        
        DataLoader loader;
        bool result = loader.loadFromFile("nonexistent.json");
        
        assert(result == false);
        
        std::cout << "✓ Invalid file test passed" << std::endl;
    }
    
    void runAllTests() {
        std::cout << "=== DataLoader Tests ===" << std::endl;
        
        try {
            testCSVLoading();
            testJSONLoading();
            testNestedJSONLoading();
            testInvalidFile();
            
            std::cout << "All DataLoader tests passed!" << std::endl;
            
        } catch (const std::exception& e) {
            std::cout << "Test failed: " << e.what() << std::endl;
            throw;
        }
    }
};

int main() {
    try {
        utils::enableUTF8Console();
        
        TestDataLoader test;
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
