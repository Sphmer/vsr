# VSR C++ - Terminal Data Visualizer

**Version: 0.9.2**

A cross-platform C++ port of the VSR terminal data visualizer with full feature parity to the Python version.

## Features

- **Multi-format Support**: Load and visualize JSON and CSV data files
- **Multiple View Modes**: Table, bar chart, tree, and mixed views
- **Interactive Navigation**: Keyboard-driven interface with scrolling and slide navigation
- **Configuration Management**: Persistent user preferences for data representation
- **Cross-platform**: Runs on Windows, macOS, and Linux
- **Multiple Data Sets**: Handle complex JSON with nested arrays and flat structures
- **Column Selection**: Choose which columns to display and in what order
- **Slide Organization**: Organize different data sets into slides for presentation

## Build Requirements

### Windows
- **MinGW-w64** or **Visual Studio 2019+**
- **CMake 3.15+** (recommended) or direct compilation
- **C++17** standard support

### macOS
- **Xcode Command Line Tools** or **Clang 10+**
- **CMake 3.15+**
- **C++17** standard support

### Linux
- **GCC 8+** or **Clang 10+**
- **CMake 3.15+**
- **C++17** standard support

## Building

### Using CMake (Recommended)

```bash
# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
make

# On Windows with MinGW
cmake .. -G "MinGW Makefiles"
mingw32-make

# On Windows with Visual Studio
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
```

### Direct Compilation

#### Windows (MinGW-w64)
```bash
g++ -std=c++17 -O2 -I./include -o vsr.exe src/*.cpp
```

#### macOS/Linux
```bash
g++ -std=c++17 -O2 -I./include -o VSR src/*.cpp
```

## Usage

```bash
# Basic usage
./VSR data.json
./VSR data.csv

# Examples with provided sample data
./VSR ../examples/sample_data.csv
./VSR ../examples/flat_data.json
./VSR ../examples/complex_data.json
```

## Controls

### Navigation
- **↑/k**: Scroll up
- **↓/j**: Scroll down  
- **←/h**: Previous slide
- **→/l**: Next slide
- **PageUp**: Scroll up one page
- **PageDown**: Scroll down one page
- **Home**: Go to top

### View Modes
- **t**: Table view
- **b**: Bar chart view
- **m**: Mixed view (default)

### Configuration
- **r**: Reconfigure data representations
- **h**: Show help
- **q**: Quit application

## Configuration

On first run, VSR will ask you to configure how each data set should be displayed:

1. **View Type**: Choose between table, bar chart, tree, or mixed view
2. **Slide Number**: Organize data sets into slides
3. **Column Selection**: Choose which columns to display and their order

Configurations are automatically saved and reused for the same data files.

## Data Format Support

### JSON Files
- **Flat Objects**: Key-value pairs displayed as single rows
- **Array of Objects**: Each object becomes a table row
- **Nested Structure**: Multiple arrays become separate data sets

### CSV Files
- **Header Row**: First row used as column names
- **Data Rows**: Subsequent rows as table data
- **Automatic Type Detection**: Numbers, text, and boolean values

## Example Data Structures

### Flat JSON
```json
{
  "New York": 8419000,
  "Los Angeles": 3980000,
  "Chicago": 2716000
}
```

### Array JSON
```json
[
  {"name": "John", "age": 30, "city": "New York"},
  {"name": "Jane", "age": 25, "city": "Los Angeles"}
]
```

### Nested JSON
```json
{
  "users": [
    {"name": "John", "age": 30},
    {"name": "Jane", "age": 25}
  ],
  "products": [
    {"name": "Laptop", "price": 999.99},
    {"name": "Mouse", "price": 29.99}
  ]
}
```

### CSV
```csv
name,population,state
New York,8419000,NY
Los Angeles,3980000,CA
Chicago,2716000,IL
```

## Project Structure

```
cpp/
├── CMakeLists.txt          # Build configuration
├── README.md               # This file
├── .gitignore             # Git ignore rules
├── include/               # Header files
│   ├── vsr_app.h         # Main application class
│   ├── data_loader.h     # Data loading functionality
│   ├── data_processor.h  # Data processing and statistics
│   ├── config_manager.h  # Configuration management
│   ├── display_manager.h # Terminal display and rendering
│   ├── input_handler.h   # Keyboard input handling
│   ├── utils.h           # Utility functions
│   └── json.hpp          # JSON parsing library
├── src/                  # Source files
│   ├── main.cpp          # Application entry point
│   ├── vsr_app.cpp       # Main application logic
│   ├── data_loader.cpp   # Data loading implementation
│   ├── data_processor.cpp # Data processing implementation
│   ├── config_manager.cpp # Configuration management
│   ├── display_manager.cpp # Display rendering
│   ├── input_handler.cpp # Input handling
│   ├── utils.cpp         # Utility implementations
│   └── test_simple.cpp   # Simple test program
├── tests/                # Test suite
│   ├── test_data_loader.cpp
│   ├── test_utils.cpp
│   └── test_display.cpp
└── build/                # Build output directory
```

## Dependencies

The C++ version uses minimal external dependencies for security and portability:

- **nlohmann/json**: Header-only JSON library (included)
- **Standard C++17 Library**: For all core functionality
- **Platform APIs**: Windows Console API, Unix termios for cross-platform support

## Troubleshooting

### Runtime Issues on Windows

If the executable runs but produces no output:

1. **Console Encoding**: The application automatically enables UTF-8 console output
2. **Runtime Libraries**: Ensure MinGW runtime DLLs are in PATH or use static linking:
   ```bash
   g++ -static -std=c++17 -O2 -I./include -o vsr.exe src/*.cpp
   ```
3. **Alternative Execution**: Try running from Command Prompt instead of PowerShell
4. **Visual Studio**: Consider using Visual Studio compiler instead of MinGW

### Build Issues

1. **Missing CMake**: Install CMake or use direct g++ compilation
2. **C++17 Support**: Ensure your compiler supports C++17 standard
3. **Include Paths**: Verify include directory paths are correct

### Data Loading Issues

1. **File Permissions**: Ensure read access to data files
2. **File Encoding**: Use UTF-8 encoding for JSON files
3. **CSV Format**: Ensure proper comma separation and header row

## Testing

Run the test suite to verify functionality:

```bash
# Build and run tests
cd build
make test

# Or run individual test executables
./test_data_loader
./test_utils
./test_display
```

## Contributing

1. Follow C++17 standards and best practices
2. Maintain cross-platform compatibility
3. Add tests for new functionality
4. Update documentation for new features
5. Ensure minimal external dependencies

## License

This project maintains the same license as the original VSR Python implementation.

## Version History

- **v0.9.2**: Added optimized terminal resize detection (SIGWINCH on Unix, polling on Windows)
- **v0.9.1**: Initial C++ port with full feature parity
- **v0.9.0**: Python version baseline

## Performance Notes

The C++ version provides significant performance improvements over the Python version:

- **Faster File Loading**: Native file I/O and parsing
- **Reduced Memory Usage**: Efficient data structures
- **Responsive UI**: Native terminal handling
- **Quick Startup**: No interpreter overhead

## Platform-Specific Notes

### Windows
- Supports both MinGW-w64 and Visual Studio compilers
- Automatic UTF-8 console configuration
- Windows Console API for terminal size detection

### macOS
- Uses Xcode Command Line Tools
- Terminal size detection via ioctl
- Native Unicode support

### Linux
- Compatible with major distributions
- Uses standard POSIX terminal APIs
- Efficient memory management
