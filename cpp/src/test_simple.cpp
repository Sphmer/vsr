#include <iostream>
#include <string>
#include "utils.h"

int main() {
    try {
        // Enable UTF-8 console output to address runtime issues
        utils::enableUTF8Console();
        
        // Force console output to be unbuffered
        std::cout.setf(std::ios::unitbuf);
        std::cerr.setf(std::ios::unitbuf);
        
        // Test basic console output
        std::cout << "VSR C++ Test Program" << std::endl;
        std::cout << "===================" << std::endl;
        
        // Test platform detection
        std::cout << "Platform: " << utils::getPlatformName() << std::endl;
        
        // Test console size detection
        auto console_size = utils::getConsoleSize();
        std::cout << "Console size: " << console_size.first << "x" << console_size.second << std::endl;
        
        // Test string utilities
        std::string test_str = "  Hello World  ";
        std::cout << "Original: '" << test_str << "'" << std::endl;
        std::cout << "Trimmed: '" << utils::trim(test_str) << "'" << std::endl;
        std::cout << "Uppercase: '" << utils::toUpper(test_str) << "'" << std::endl;
        
        // Test numeric utilities
        std::string num_str = "123.45";
        std::cout << "Is numeric '" << num_str << "': " << (utils::isNumeric(num_str) ? "true" : "false") << std::endl;
        std::cout << "As double: " << utils::toDouble(num_str) << std::endl;
        
        // Test file utilities
        std::cout << "Current directory exists: " << (utils::directoryExists(".") ? "true" : "false") << std::endl;
        
        // Test timestamp
        std::cout << "Current timestamp: " << utils::getCurrentTimestamp() << std::endl;
        
        std::cout << std::endl;
        std::cout << "All tests completed successfully!" << std::endl;
        std::cout << "Press any key to exit..." << std::endl;
        
        // Wait for input to keep console open
        std::cin.get();
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        std::cin.get();
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        std::cin.get();
        return 1;
    }
}
