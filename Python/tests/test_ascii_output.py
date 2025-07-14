#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Visual Output Test - Demonstrates VSR's table and bar chart rendering
"""

import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from vsr import VSRApp

# Set UTF-8 encoding for Windows
if os.name == 'nt':
    try:
        import codecs
        if hasattr(sys.stdout, 'reconfigure'):
            sys.stdout.reconfigure(encoding='utf-8')
        else:
            sys.stdout = codecs.getwriter('utf-8')(sys.stdout.buffer)
    except (AttributeError, OSError):
        # Fallback for older Python versions or systems without UTF-8 support
        pass

def print_separator(title):
    """Print a formatted separator with title"""
    print(f"\n{'='*60}")
    print(f"{title:^60}")
    print(f"{'='*60}")

def test_json_table_output():
    """Test and display JSON table output"""
    print_separator("JSON DATA - TABLE VIEW")
    app = VSRApp("../examples/flat_data.json")
    app.data = app.load_data()
    app.processed_data = app.process_data(app.data)
    app.terminal_width = 80
    app.terminal_height = 24
    app.max_display_rows = 10
    app.view_mode = "table"
    
    table_output = app.create_table_view()
    print(table_output)
    print(f"\n📊 Displayed {len(app.processed_data)} rows in table format")

def test_json_bar_output():
    """Test and display JSON bar output"""
    print_separator("JSON DATA - BAR CHART VIEW")
    app = VSRApp("../examples/flat_data.json")
    app.data = app.load_data()
    app.processed_data = app.process_data(app.data)
    app.terminal_width = 80
    app.terminal_height = 24
    app.max_display_rows = 10
    app.view_mode = "bars"
    
    bar_output = app.create_bar_view()
    print(bar_output)
    print(f"\n📊 Displayed {len(app.processed_data)} bars using █ (filled) and ▒ (empty) characters")

def test_csv_table_output():
    """Test CSV table data"""
    print_separator("CSV DATA - TABLE VIEW")
    app = VSRApp("../examples/sample_data.csv")
    app.data = app.load_data()
    app.processed_data = app.process_data(app.data)
    app.terminal_width = 80
    app.terminal_height = 24
    app.max_display_rows = 10
    app.view_mode = "table"
    
    table_output = app.create_table_view()
    print(table_output)
    print(f"\n📊 Displayed {len(app.processed_data)} rows from CSV file")

def test_csv_bar_output():
    """Test CSV bar chart"""
    print_separator("CSV DATA - BAR CHART VIEW")
    app = VSRApp("../examples/sample_data.csv")
    app.data = app.load_data()
    app.processed_data = app.process_data(app.data)
    app.terminal_width = 80
    app.terminal_height = 24
    app.max_display_rows = 10
    app.view_mode = "bars"
    
    bar_output = app.create_bar_view()
    print(bar_output)
    print(f"\n📊 Displayed {len(app.processed_data)} bars with intelligent numeric field detection")

def test_complex_json_output():
    """Test complex JSON data if available"""
    try:
        print_separator("COMPLEX JSON DATA - TABLE VIEW")
        app = VSRApp("../examples/complex_data.json")
        app.data = app.load_data()
        app.processed_data = app.process_data(app.data)
        app.terminal_width = 80
        app.terminal_height = 24
        app.max_display_rows = 10
        app.view_mode = "table"
        
        table_output = app.create_table_view()
        print(table_output)
        print(f"\n📊 Processed complex JSON with {len(app.processed_data)} items")
        
        print_separator("COMPLEX JSON DATA - BAR CHART VIEW")
        app.view_mode = "bars"
        bar_output = app.create_bar_view()
        print(bar_output)
        print(f"\n📊 Generated bar chart from complex JSON structure")
        
    except FileNotFoundError:
        print_separator("COMPLEX JSON DATA - SKIPPED")
        print("⚠️  Complex data file not found, skipping this test")

def test_terminal_sizing():
    """Test different terminal sizes"""
    print_separator("TERMINAL SIZING TEST")
    app = VSRApp("../examples/flat_data.json")
    app.data = app.load_data()
    app.processed_data = app.process_data(app.data)
    app.view_mode = "table"
    
    # Test different terminal widths
    sizes = [(60, 20), (100, 30), (120, 40)]
    
    for width, height in sizes:
        print(f"\n--- Terminal Size: {width}x{height} ---")
        app.terminal_width = width
        app.terminal_height = height
        app.max_display_rows = height - 8
        
        table_output = app.create_table_view()
        # Show first few lines to demonstrate sizing
        lines = table_output.split('\n')[:5]
        for line in lines:
            print(line)
        print(f"... (showing first 5 lines, actual width: {len(lines[0]) if lines else 0})")

def run_visual_tests():
    """Run all visual output tests"""
    print("🎨 VSR Visual Output Test Suite")
    print("This script demonstrates the visual output capabilities of VSR\n")
    
    tests = [
        test_json_table_output,
        test_json_bar_output,
        test_csv_table_output,
        test_csv_bar_output,
        test_complex_json_output,
        test_terminal_sizing
    ]
    
    for i, test in enumerate(tests, 1):
        try:
            test()
        except Exception as e:
            print(f"\n❌ Test {i} failed: {e}")
        
        # Add spacing between tests
        if i < len(tests):
            print("\n" + "-"*60)
    
    print_separator("VISUAL TESTS COMPLETED")
    print("🎉 All visual output tests completed!")
    print("\nThese examples show how VSR renders:")
    print("  • ASCII tables with proper alignment and borders")
    print("  • Bar charts using █ (filled) and ▒ (empty) block characters")
    print("  • Intelligent data processing for both JSON and CSV formats")
    print("  • Responsive layout that adapts to different terminal sizes")

if __name__ == "__main__":
    run_visual_tests()
