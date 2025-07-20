#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include "vsr_app.h"
#include "utils.h"

// Version information
const std::string VSR_VERSION = "0.9.1";

void printUsage() {
    std::cout << "VSR - A minimalistic terminal data visualizer\n";
    std::cout << "Version: " << VSR_VERSION << "\n";
    std::cout << "Usage: vsr <file.json|file.csv>\n";
    std::cout << "\nSupported formats:\n";
    std::cout << "  - JSON files (.json)\n";
    std::cout << "  - CSV files (.csv)\n";
    std::cout << "\nExamples:\n";
    std::cout << "  vsr data.json\n";
    std::cout << "  vsr sample.csv\n";
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    try {
        // Enable UTF-8 console output on Windows
        utils::enableUTF8Console();
        
        // Force console output to be unbuffered to prevent issues
        std::cout.setf(std::ios::unitbuf);
        std::cerr.setf(std::ios::unitbuf);
        
        // Print startup message to verify console output works
        std::cout << "VSR C++ v" << VSR_VERSION << " starting..." << std::endl;
        
        // Parse command line arguments
        if (argc < 2) {
            printUsage();
            return 1;
        }
        
        std::string filename = argv[1];
        
        // Check if file exists
        if (!utils::fileExists(filename)) {
            std::cerr << "Error: File '" << filename << "' not found." << std::endl;
            return 1;
        }
        
        // Check file extension
        std::string ext = utils::getFileExtension(filename);
        if (ext != ".json" && ext != ".csv") {
            std::cerr << "Error: Unsupported file format '" << ext << "'. Supported formats: .json, .csv" << std::endl;
            return 1;
        }
        
        std::cout << "Loading file: " << filename << std::endl;
        
        // Create and run VSR application
        VSRApp app(filename);
        
        if (!app.initialize()) {
            std::cerr << "Error: Failed to initialize VSR application." << std::endl;
            return 1;
        }
        
        std::cout << "VSR initialized successfully. Starting main loop..." << std::endl;
        
        // Run the main application loop
        app.run();
        
        // Clean shutdown
        app.shutdown();
        
        std::cout << "VSR exited successfully." << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Error: Unknown exception occurred." << std::endl;
        return 1;
    }
}
