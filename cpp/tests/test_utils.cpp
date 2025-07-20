#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include "../include/utils.h"

class TestUtils {
public:
    void testStringUtilities() {
        std::cout << "Testing string utilities..." << std::endl;
        
        // Test trim
        assert(utils::trim("  hello  ") == "hello");
        assert(utils::trim("") == "");
        assert(utils::trim("   ") == "");
        
        // Test case conversion
        assert(utils::toLower("HELLO") == "hello");
        assert(utils::toUpper("hello") == "HELLO");
        assert(utils::toLower("MiXeD") == "mixed");
        
        // Test split
        auto parts = utils::split("a,b,c", ",");
        assert(parts.size() == 3);
        assert(parts[0] == "a");
        assert(parts[1] == "b");
        assert(parts[2] == "c");
        
        // Test join
        std::vector<std::string> words = {"hello", "world", "test"};
        assert(utils::join(words, " ") == "hello world test");
        assert(utils::join(words, ",") == "hello,world,test");
        
        // Test startsWith and endsWith
        assert(utils::startsWith("hello world", "hello") == true);
        assert(utils::startsWith("hello world", "world") == false);
        assert(utils::endsWith("hello world", "world") == true);
        assert(utils::endsWith("hello world", "hello") == false);
        
        // Test replaceAll
        assert(utils::replaceAll("hello world hello", "hello", "hi") == "hi world hi");
        
        std::cout << "✓ String utilities test passed" << std::endl;
    }
    
    void testNumericUtilities() {
        std::cout << "Testing numeric utilities..." << std::endl;
        
        // Test isNumeric
        assert(utils::isNumeric("123") == true);
        assert(utils::isNumeric("123.45") == true);
        assert(utils::isNumeric("-123.45") == true);
        assert(utils::isNumeric("hello") == false);
        assert(utils::isNumeric("") == false);
        
        // Test conversions
        assert(utils::toDouble("123.45") == 123.45);
        assert(utils::toInt("123") == 123);
        assert(utils::toDouble("invalid") == 0.0);
        assert(utils::toInt("invalid") == 0);
        
        // Test formatting
        assert(utils::formatNumber(123.456, 2) == "123.46");
        assert(utils::formatInteger(123) == "123");
        
        std::cout << "✓ Numeric utilities test passed" << std::endl;
    }
    
    void testFileUtilities() {
        std::cout << "Testing file utilities..." << std::endl;
        
        // Test file extension
        assert(utils::getFileExtension("test.json") == ".json");
        assert(utils::getFileExtension("test.csv") == ".csv");
        assert(utils::getFileExtension("test") == "");
        
        // Test filename
        assert(utils::getFileName("/path/to/test.json") == "test.json");
        assert(utils::getFileName("test.json") == "test.json");
        
        // Test directory existence (current directory should exist)
        assert(utils::directoryExists(".") == true);
        assert(utils::directoryExists("nonexistent_directory") == false);
        
        std::cout << "✓ File utilities test passed" << std::endl;
    }
    
    void testPlatformDetection() {
        std::cout << "Testing platform detection..." << std::endl;
        
        // At least one platform should be detected
        bool platform_detected = utils::isWindows() || utils::isMacOS() || utils::isLinux();
        assert(platform_detected == true);
        
        // Platform name should not be empty
        std::string platform_name = utils::getPlatformName();
        assert(!platform_name.empty());
        assert(platform_name != "Unknown");
        
        std::cout << "✓ Platform detection test passed (Platform: " << platform_name << ")" << std::endl;
    }
    
    void testConsoleUtilities() {
        std::cout << "Testing console utilities..." << std::endl;
        
        // Test console size detection
        auto size = utils::getConsoleSize();
        assert(size.first > 0);  // Width should be positive
        assert(size.second > 0); // Height should be positive
        
        std::cout << "✓ Console utilities test passed (Size: " << size.first << "x" << size.second << ")" << std::endl;
    }
    
    void testDataConversion() {
        std::cout << "Testing data conversion..." << std::endl;
        
        // Test anyToString with different types
        std::any str_val = std::string("hello");
        std::any int_val = 42;
        std::any double_val = 3.14;
        std::any bool_val = true;
        
        assert(utils::anyToString(str_val) == "hello");
        assert(utils::anyToString(int_val) == "42");
        assert(utils::anyToString(double_val) == "3.14");
        assert(utils::anyToString(bool_val) == "true");
        
        // Test stringToAny
        std::any converted_bool = utils::stringToAny("true");
        std::any converted_int = utils::stringToAny("123");
        std::any converted_double = utils::stringToAny("123.45");
        std::any converted_str = utils::stringToAny("hello");
        
        // These should not throw exceptions
        utils::anyToString(converted_bool);
        utils::anyToString(converted_int);
        utils::anyToString(converted_double);
        utils::anyToString(converted_str);
        
        std::cout << "✓ Data conversion test passed" << std::endl;
    }
    
    void testJSONValidation() {
        std::cout << "Testing JSON validation..." << std::endl;
        
        // Test valid JSON
        assert(utils::isValidJSON("{\"key\": \"value\"}") == true);
        assert(utils::isValidJSON("[1, 2, 3]") == true);
        
        // Test invalid JSON
        assert(utils::isValidJSON("not json") == false);
        assert(utils::isValidJSON("") == false);
        
        std::cout << "✓ JSON validation test passed" << std::endl;
    }
    
    void testHashUtilities() {
        std::cout << "Testing hash utilities..." << std::endl;
        
        // Test MD5 calculation
        std::string hash1 = utils::calculateMD5("hello");
        std::string hash2 = utils::calculateMD5("hello");
        std::string hash3 = utils::calculateMD5("world");
        
        // Same input should produce same hash
        assert(hash1 == hash2);
        // Different input should produce different hash
        assert(hash1 != hash3);
        // Hash should not be empty
        assert(!hash1.empty());
        
        std::cout << "✓ Hash utilities test passed" << std::endl;
    }
    
    void testTimeUtilities() {
        std::cout << "Testing time utilities..." << std::endl;
        
        // Test timestamp generation
        std::string timestamp = utils::getCurrentTimestamp();
        assert(!timestamp.empty());
        
        // Timestamp should contain date and time components
        assert(timestamp.find("-") != std::string::npos); // Date separator
        assert(timestamp.find(":") != std::string::npos); // Time separator
        
        std::cout << "✓ Time utilities test passed" << std::endl;
    }
    
    void runAllTests() {
        std::cout << "=== Utils Tests ===" << std::endl;
        
        try {
            testStringUtilities();
            testNumericUtilities();
            testFileUtilities();
            testPlatformDetection();
            testConsoleUtilities();
            testDataConversion();
            testJSONValidation();
            testHashUtilities();
            testTimeUtilities();
            
            std::cout << "All Utils tests passed!" << std::endl;
            
        } catch (const std::exception& e) {
            std::cout << "Test failed: " << e.what() << std::endl;
            throw;
        }
    }
};

int main() {
    try {
        utils::enableUTF8Console();
        
        TestUtils test;
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
