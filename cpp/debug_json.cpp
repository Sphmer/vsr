#include <iostream>
#include <string>
#include "include/utils.h"
#include "include/data_loader.h"

int main() {
    // Test JSON validation
    std::string filename = "../test_sample.json";
    std::cout << "Testing file: " << filename << std::endl;
    
    // Check if file exists
    if (!utils::fileExists(filename)) {
        std::cout << "File does not exist!" << std::endl;
        return 1;
    }
    
    // Read file content
    std::string content = utils::readFile(filename);
    std::cout << "File content length: " << content.length() << std::endl;
    std::cout << "File content: " << content << std::endl;
    
    // Test JSON validation
    bool isValid = utils::isValidJSON(content);
    std::cout << "Is valid JSON: " << (isValid ? "true" : "false") << std::endl;
    
    // Test data loader
    DataLoader loader;
    bool loaded = loader.loadFromFile(filename);
    std::cout << "Loaded successfully: " << (loaded ? "true" : "false") << std::endl;
    
    return 0;
}
