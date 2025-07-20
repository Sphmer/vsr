@echo off
echo VSR C++ Build Script
echo ===================

echo.
echo Checking for available compilers...

:: Check for Visual Studio cl.exe
where cl >nul 2>&1
if %errorlevel% == 0 (
    echo Found Visual Studio compiler (cl.exe)
    goto :build_with_cl
)

:: Check for MinGW g++
where g++ >nul 2>&1
if %errorlevel% == 0 (
    echo Found MinGW g++ compiler
    goto :build_with_gcc
)

:: Check for clang
where clang++ >nul 2>&1
if %errorlevel% == 0 (
    echo Found Clang compiler
    goto :build_with_clang
)

echo No suitable C++ compiler found!
echo Please install one of the following:
echo - Visual Studio Build Tools or Community Edition
echo - MinGW-w64 (via MSYS2 or standalone)
echo - LLVM/Clang
echo.
echo You can install Visual Studio Build Tools with:
echo winget install Microsoft.VisualStudio.2019.BuildTools
echo.
echo Or install MinGW-w64 via MSYS2:
echo winget install MSYS2.MSYS2
echo.
pause
exit /b 1

:build_with_cl
echo Building with Visual Studio compiler...
if not exist build mkdir build
cd build

cl /std:c++17 /EHsc /I..\include /Fe:VSR.exe ..\src\*.cpp
if %errorlevel% == 0 (
    echo Main VSR executable built successfully!
) else (
    echo Build failed!
    pause
    exit /b 1
)

:: Build test executables
echo Building test executables...
cl /std:c++17 /EHsc /I..\include /Fe:test_simple.exe ..\src\test_simple.cpp ..\src\utils.cpp
cl /std:c++17 /EHsc /I..\include /Fe:test_utils.exe ..\tests\test_utils.cpp ..\src\utils.cpp
cl /std:c++17 /EHsc /I..\include /Fe:test_data_loader.exe ..\tests\test_data_loader.cpp ..\src\data_loader.cpp ..\src\utils.cpp

cd ..
goto :test_executables

:build_with_gcc
echo Building with MinGW g++ compiler...
if not exist build mkdir build
cd build

g++ -std=c++17 -O2 -I..\include -o VSR.exe ..\src\*.cpp
if %errorlevel% == 0 (
    echo Main VSR executable built successfully!
) else (
    echo Build failed!
    pause
    exit /b 1
)

:: Build test executables
echo Building test executables...
g++ -std=c++17 -I..\include -o test_simple.exe ..\src\test_simple.cpp ..\src\utils.cpp
g++ -std=c++17 -I..\include -o test_utils.exe ..\tests\test_utils.cpp ..\src\utils.cpp
g++ -std=c++17 -I..\include -o test_data_loader.exe ..\tests\test_data_loader.cpp ..\src\data_loader.cpp ..\src\utils.cpp

cd ..
goto :test_executables

:build_with_clang
echo Building with Clang compiler...
if not exist build mkdir build
cd build

clang++ -std=c++17 -O2 -I..\include -o VSR.exe ..\src\*.cpp
if %errorlevel% == 0 (
    echo Main VSR executable built successfully!
) else (
    echo Build failed!
    pause
    exit /b 1
)

cd ..
goto :test_executables

:test_executables
echo.
echo Build completed! Testing executables...
echo.

cd build

:: Test simple executable
if exist test_simple.exe (
    echo Running test_simple.exe...
    test_simple.exe
    echo.
) else (
    echo test_simple.exe not found
)

:: Test main VSR executable
if exist VSR.exe (
    echo Testing main VSR executable...
    echo VSR.exe --help
    VSR.exe --help
    echo.
    
    echo Testing with sample data...
    if exist "..\..\examples\sample_data.csv" (
        echo VSR.exe ..\..\examples\sample_data.csv
        VSR.exe ..\..\examples\sample_data.csv
    ) else (
        echo Sample data not found at ..\..\examples\sample_data.csv
    )
) else (
    echo VSR.exe not found
)

cd ..

echo.
echo Build and test completed!
echo.
echo Available executables in build\ directory:
if exist build\VSR.exe echo - VSR.exe (main application)
if exist build\test_simple.exe echo - test_simple.exe (simple test)
if exist build\test_utils.exe echo - test_utils.exe (utility tests)
if exist build\test_data_loader.exe echo - test_data_loader.exe (data loader tests)

echo.
pause
