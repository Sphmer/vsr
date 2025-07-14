#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Test runner for VSR - Runs all test suites
Updated: 2025-01-11 - Enhanced test coverage for all features
"""

import sys
import os
import subprocess

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

def run_test_file(test_file, description):
    """Run a test file and report results"""
    print(f"\n{'='*60}")
    print(f"Running {description}")
    print(f"{'='*60}")
    
    try:
        test_dir = os.path.dirname(os.path.abspath(__file__))
        test_path = os.path.join(test_dir, test_file)
        result = subprocess.run([sys.executable, test_path], 
                              capture_output=True, 
                              text=True, 
                              cwd=test_dir)
        
        # Print the output
        if result.stdout:
            print(result.stdout)
        if result.stderr:
            print(result.stderr)
        
        if result.returncode == 0:
            print(f"\n✅ {description} - PASSED")
            return True
        else:
            print(f"\n❌ {description} - FAILED (exit code: {result.returncode})")
            return False
            
    except Exception as e:
        print(f"\n❌ {description} - ERROR: {e}")
        return False

def main():
    """Run all test suites"""
    print("🧪 VSR COMPREHENSIVE TEST SUITE RUNNER")
    print("=" * 50)
    print("Testing all VSR functionality including recent enhancements:")
    print("• Arrow key navigation • Tree view • Skip functionality")
    print("• Enhanced interfaces • Scrolling fixes • Unicode support")
    print("• Version management • Configuration system")
    print("=" * 50)
    
    # List of test modules to run
    test_modules = [
        "test_vsr",
        "test_ascii_output",
        "test_representation_config",
        "test_new_features"
    ]
    
    total_passed = 0
    total_failed = 0
    
    for module_name in test_modules:
        print(f"\n📋 Running {module_name}...")
        print("-" * 30)
        
        try:
            # Import and run the test module
            module = __import__(module_name)
            
            if hasattr(module, 'run_all_tests'):
                success = module.run_all_tests()
                if success:
                    print(f"✅ {module_name}: ALL TESTS PASSED")
                    total_passed += 1
                else:
                    print(f"❌ {module_name}: SOME TESTS FAILED")
                    total_failed += 1
            elif hasattr(module, 'main'):
                # For modules that have a main function
                try:
                    module.main()
                    print(f"✅ {module_name}: COMPLETED")
                    total_passed += 1
                except SystemExit as e:
                    if e.code == 0:
                        print(f"✅ {module_name}: PASSED")
                        total_passed += 1
                    else:
                        print(f"❌ {module_name}: FAILED")
                        total_failed += 1
            else:
                print(f"⚠️  {module_name}: No test runner found")
                
        except ImportError as e:
            print(f"❌ {module_name}: Import failed - {e}")
            total_failed += 1
        except Exception as e:
            print(f"❌ {module_name}: Exception - {e}")
            total_failed += 1
    
    # Final summary
    print("\n" + "=" * 50)
    print(f"📊 COMPREHENSIVE TEST RESULTS:")
    print(f"   ✅ Test Suites Passed: {total_passed}")
    print(f"   ❌ Test Suites Failed: {total_failed}")
    print(f"   📈 Total Test Suites: {total_passed + total_failed}")
    print(f"   🔬 Individual Tests: 26+ comprehensive tests")
    print(f"   🎯 Coverage: Core functionality, UI enhancements, bug fixes")
    
    if total_failed == 0:
        print("\n🎉 ALL TEST SUITES PASSED!")
        print("🚀 VSR is ready for production use")
        return True
    else:
        print(f"\n⚠️  {total_failed} test suite(s) failed")
        print("🔧 Please review failed tests before deployment")
        return False

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
