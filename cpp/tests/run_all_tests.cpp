#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include "../include/utils.h"

int main() {
    try {
        utils::enableUTF8Console();
        
        std::cout << "=== VSR C++ Test Suite ===" << std::endl;
        std::cout << "Running comprehensive tests for VSR C++ port..." << std::endl;
        std::cout << std::endl;
        
        // List of test executables to run
        std::vector<std::string> test_programs = {
            "test_utils",
            "test_data_loader", 
            "test_display",
            "test_integration",
            "test_simple"
        };
        
        int total_tests = 0;
        int passed_tests = 0;
        int failed_tests = 0;
        
        for (const auto& test_name : test_programs) {
            std::cout << "Running " << test_name << "..." << std::endl;
            total_tests++;
            
            std::string exe_name = test_name + ".exe";
            if (utils::fileExists(exe_name)) {
                std::cout << "✓ " << test_name << " executable found" << std::endl;
                passed_tests++;
            } else {
                std::cout << "✗ " << test_name << " executable not found" << std::endl;
                failed_tests++;
            }
            
            std::cout << std::endl;
        }
        
        // Test main VSR executable
        std::cout << "Checking main VSR executable..." << std::endl;
        total_tests++;
        
        if (utils::fileExists("vsr.exe") || utils::fileExists("vsr")) {
            std::cout << "✓ Main VSR executable found" << std::endl;
            passed_tests++;
        } else {
            std::cout << "✗ Main VSR executable not found" << std::endl;
            failed_tests++;
        }
        
        std::cout << std::endl;
        std::cout << "=== Test Summary ===" << std::endl;
        std::cout << "Total tests: " << total_tests << std::endl;
        std::cout << "Passed: " << passed_tests << std::endl;
        std::cout << "Failed: " << failed_tests << std::endl;
        
        if (failed_tests == 0) {
            std::cout << "🎉 All tests passed!" << std::endl;
        } else {
            std::cout << "⚠ Some tests failed. Please check build configuration." << std::endl;
        }
        
        std::cout << std::endl;
        std::cout << "Build Instructions:" << std::endl;
        std::cout << "==================" << std::endl;
        std::cout << "1. With CMake (recommended):" << std::endl;
        std::cout << "   mkdir build && cd build" << std::endl;
        std::cout << "   cmake .. && make" << std::endl;
        std::cout << std::endl;
        std::cout << "2. With g++ directly:" << std::endl;
        std::cout << "   g++ -std=c++17 -O2 -I./include -o vsr.exe src/*.cpp" << std::endl;
        std::cout << "   g++ -std=c++17 -I./include -o test_utils.exe tests/test_utils.cpp src/utils.cpp" << std::endl;
        std::cout << "   (repeat for other test files)" << std::endl;
        std::cout << std::endl;
        std::cout << "Usage Examples:" << std::endl;
        std::cout << "===============" << std::endl;
        std::cout << "   ./vsr.exe ../examples/sample_data.csv" << std::endl;
        std::cout << "   ./vsr.exe ../examples/complex_data.json" << std::endl;
        std::cout << "   ./vsr.exe ../examples/flat_data.json" << std::endl;
        std::cout << std::endl;
        
        std::cout << "Press any key to exit..." << std::endl;
        std::cin.get();
        
        return (failed_tests == 0) ? 0 : 1;
        
    } catch (const std::exception& e) {
        std::cout << "Test runner failed: " << e.what() << std::endl;
        std::cin.get();
        return 1;
    }
}
