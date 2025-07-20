cmake_minimum_required(VERSION 3.16)
project(VSR VERSION 0.9.1 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4 /utf-8)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find required packages
find_package(Threads REQUIRED)

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
    src/vsr_app.cpp
    src/data_loader.cpp
    src/data_processor.cpp
    src/config_manager.cpp
    src/display_manager.cpp
    src/input_handler.cpp
    src/utils.cpp
)

# Header files
set(HEADERS
    include/vsr_app.h
    include/data_loader.h
    include/data_processor.h
    include/config_manager.h
    include/display_manager.h
    include/input_handler.h
    include/utils.h
    include/json.hpp
)

# Create executable
add_executable(vsr ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(vsr Threads::Threads)

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    target_compile_definitions(vsr PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
elseif(APPLE)
    # macOS-specific settings
    target_compile_definitions(vsr PRIVATE __APPLE__)
else()
    # Linux-specific settings
    target_compile_definitions(vsr PRIVATE __LINUX__)
endif()

# Install target
install(TARGETS vsr DESTINATION bin)

# Enable testing
enable_testing()

# Test executables
add_executable(test_simple src/test_simple.cpp src/utils.cpp)
target_include_directories(test_simple PRIVATE include)
target_link_libraries(test_simple ${CMAKE_THREAD_LIBS_INIT})

add_executable(test_utils tests/test_utils.cpp src/utils.cpp)
target_include_directories(test_utils PRIVATE include)
target_link_libraries(test_utils ${CMAKE_THREAD_LIBS_INIT})

add_executable(test_data_loader tests/test_data_loader.cpp src/data_loader.cpp src/utils.cpp)
target_include_directories(test_data_loader PRIVATE include)
target_link_libraries(test_data_loader ${CMAKE_THREAD_LIBS_INIT})

add_executable(test_display tests/test_display.cpp src/display_manager.cpp src/data_loader.cpp src/data_processor.cpp src/utils.cpp)
target_include_directories(test_display PRIVATE include)
target_link_libraries(test_display ${CMAKE_THREAD_LIBS_INIT})

add_executable(test_integration tests/test_integration.cpp ${SOURCES})
target_include_directories(test_integration PRIVATE include)
target_link_libraries(test_integration ${CMAKE_THREAD_LIBS_INIT})

add_executable(run_all_tests tests/run_all_tests.cpp src/utils.cpp)
target_include_directories(run_all_tests PRIVATE include)
target_link_libraries(run_all_tests ${CMAKE_THREAD_LIBS_INIT})

add_executable(vsr_test
    tests/test_main.cpp
    tests/test_data_loader.cpp
    tests/test_data_processor.cpp
    tests/test_config_manager.cpp
    tests/test_display_manager.cpp
    src/data_loader.cpp
    src/data_processor.cpp
    src/config_manager.cpp
    src/display_manager.cpp
    src/utils.cpp
)

target_link_libraries(vsr_test Threads::Threads)
add_test(NAME vsr_tests COMMAND vsr_test)

# Copy examples for testing
file(COPY ${CMAKE_SOURCE_DIR}/../examples DESTINATION ${CMAKE_BINARY_DIR})
