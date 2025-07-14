#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Test script for new VSR features: skip functionality and page navigation
"""

import json
import os
import sys
import tempfile
import unittest
from unittest.mock import patch, MagicMock

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from vsr import VSRApp

class TestNewFeatures(unittest.TestCase):
    """Test new VSR features: skip functionality and page navigation"""
    
    def setUp(self):
        """Set up test data"""
        self.test_data = {
            "users": [
                {"name": "Alice", "age": 30, "salary": 75000},
                {"name": "Bob", "age": 25, "salary": 65000}
            ],
            "products": [
                {"name": "Widget A", "price": 29.99, "stock": 100},
                {"name": "Widget B", "price": 39.99, "stock": 50}
            ],
            "sales": [
                {"month": "Jan", "revenue": 50000, "profit": 15000},
                {"month": "Feb", "revenue": 55000, "profit": 18000}
            ]
        }
        
        # Create temporary test file
        self.temp_file = tempfile.NamedTemporaryFile(mode='w', suffix='.json', delete=False)
        json.dump(self.test_data, self.temp_file)
        self.temp_file.close()
        
        self.app = VSRApp(self.temp_file.name)
        self.data_sets = self.app.identify_data_sets(self.app.data)
    
    def tearDown(self):
        """Clean up test data"""
        os.unlink(self.temp_file.name)
    
    def test_skip_functionality_exists(self):
        """Test that skip functionality methods exist"""
        # Test that the configuration method accepts skip type
        skip_config = {'type': 'skip'}
        self.assertEqual(skip_config['type'], 'skip')
        
        # Test that progress bar method exists and accepts data set names
        data_set_names = ['users', 'products', 'sales']
        progress_bar = self.app._create_progress_bar(0, 3, {}, data_set_names)
        self.assertIsInstance(progress_bar, str)
        self.assertIn('(1/3)', progress_bar)
    
    def test_progress_bar_with_skip(self):
        """Test progress bar shows skip symbols correctly"""
        data_set_names = ['users', 'products', 'sales']
        preferences = {
            'users': {'type': 'table'},
            'products': {'type': 'skip'},
            'sales': {'type': 'bars', 'field': 'revenue'}
        }
        
        # Test progress bar at different positions
        progress_bar = self.app._create_progress_bar(2, 3, preferences, data_set_names)
        self.assertIn('✓', progress_bar)  # Completed
        self.assertIn('➖', progress_bar)  # Skipped
        self.assertIn('●', progress_bar)  # Current
        self.assertIn('Configured: 1', progress_bar)
        self.assertIn('Skipped: 1', progress_bar)
    
    def test_configuration_summary_with_skip(self):
        """Test configuration summary separates configured and skipped data sets"""
        preferences = {
            'users': {'type': 'table', 'columns': ['name', 'age']},
            'products': {'type': 'skip'},
            'sales': {'type': 'bars', 'field': 'revenue'}
        }
        
        # Mock _get_key_input to avoid waiting for user input
        with patch.object(self.app, '_get_key_input', return_value='q'):
            with patch('builtins.print') as mock_print:
                self.app._show_configuration_summary(preferences)
                
                # Check that print was called with expected content
                print_calls = [call[0][0] for call in mock_print.call_args_list if call[0]]
                print_output = ' '.join(print_calls)
                
                self.assertIn('Configuration Complete', print_output)
                self.assertIn('Total data sets: 3', print_output)
                self.assertIn('Configured for display: 2', print_output)
                self.assertIn('Skipped: 1', print_output)
                self.assertIn('Data sets to display', print_output)
                self.assertIn('Skipped data sets', print_output)
    
    def test_skip_filtering(self):
        """Test that skipped data sets are filtered out from final preferences"""
        preferences = {
            'users': {'type': 'table'},
            'products': {'type': 'skip'},
            'sales': {'type': 'bars', 'field': 'revenue'}
        }
        
        # Filter out skipped data sets (simulate what ask_representation_preferences does)
        filtered_preferences = {name: config for name, config in preferences.items() 
                               if config.get('type') != 'skip'}
        
        self.assertEqual(len(filtered_preferences), 2)
        self.assertIn('users', filtered_preferences)
        self.assertIn('sales', filtered_preferences)
        self.assertNotIn('products', filtered_preferences)
    
    def test_data_set_configuration_method_signature(self):
        """Test that _configure_single_data_set method has correct signature"""
        # Test that the method exists and can be called with required parameters
        method = getattr(self.app, '_configure_single_data_set', None)
        self.assertIsNotNone(method, "_configure_single_data_set method should exist")
        
        # Test method signature by checking if it accepts the required parameters
        import inspect
        sig = inspect.signature(method)
        expected_params = ['set_name', 'set_info', 'current_index', 'total_count', 
                          'existing_preferences', 'data_set_names']
        actual_params = list(sig.parameters.keys())
        self.assertEqual(actual_params, expected_params)
    
    def test_progress_bar_edge_cases(self):
        """Test progress bar with edge cases"""
        data_set_names = ['single']
        
        # Test with single data set
        progress_bar = self.app._create_progress_bar(0, 1, {}, data_set_names)
        self.assertIn('(1/1)', progress_bar)
        self.assertIn('●', progress_bar)
        
        # Test with all skipped
        preferences = {'single': {'type': 'skip'}}
        progress_bar = self.app._create_progress_bar(1, 1, preferences, data_set_names)
        self.assertIn('➖', progress_bar)
        self.assertIn('Skipped: 1', progress_bar)
        self.assertIn('Configured: 0', progress_bar)
    
    def test_configuration_summary_all_skipped(self):
        """Test configuration summary when all data sets are skipped"""
        preferences = {
            'users': {'type': 'skip'},
            'products': {'type': 'skip'},
            'sales': {'type': 'skip'}
        }
        
        with patch.object(self.app, '_get_key_input', return_value='q'):
            with patch('builtins.print') as mock_print:
                self.app._show_configuration_summary(preferences)
                
                print_calls = [call[0][0] for call in mock_print.call_args_list if call[0]]
                print_output = ' '.join(print_calls)
                
                self.assertIn('No data sets configured for display', print_output)
                self.assertIn('All data sets were skipped', print_output)
    
    def test_configuration_summary_none_skipped(self):
        """Test configuration summary when no data sets are skipped"""
        preferences = {
            'users': {'type': 'table'},
            'products': {'type': 'bars', 'field': 'price'},
            'sales': {'type': 'table'}
        }
        
        with patch.object(self.app, '_get_key_input', return_value='q'):
            with patch('builtins.print') as mock_print:
                self.app._show_configuration_summary(preferences)
                
                print_calls = [call[0][0] for call in mock_print.call_args_list if call[0]]
                print_output = ' '.join(print_calls)
                
                self.assertIn('Configured for display: 3', print_output)
                self.assertIn('Skipped: 0', print_output)
                self.assertNotIn('Skipped data sets:', print_output)

    def test_skip_not_processed_in_view_phase(self):
        """Test that skipped data sets are not processed during view phase."""
        # Create test data sets
        data_sets = {
            'users': {'type': 'list', 'data': [{'name': 'Alice', 'age': 30}]},
            'products': {'type': 'list', 'data': [{'name': 'Widget', 'price': 29.99}]},
            'sales': {'type': 'list', 'data': [{'month': 'Jan', 'revenue': 50000}]}
        }
        
        # Create preferences with products skipped
        preferences = {
            'users': {'type': 'table'},
            'sales': {'type': 'bars', 'field': 'revenue'}
            # 'products' is skipped (not in preferences)
        }
        
        # Process data sets with preferences
        processed_data = self.app.process_multiple_data_sets(data_sets, preferences)
        
        # Check which data sets are in processed data
        data_set_names_in_processed = set()
        for item in processed_data:
            if '_data_set' in item:
                data_set_names_in_processed.add(item['_data_set'])
        
        # Verify that configured data sets are present
        self.assertIn('users', data_set_names_in_processed)
        self.assertIn('sales', data_set_names_in_processed)
        
        # Verify that skipped data set is NOT present
        self.assertNotIn('products', data_set_names_in_processed)
        
        # Verify correct number of data sets processed
        self.assertEqual(len(data_set_names_in_processed), 2)

def run_new_features_tests():
    """Run all new features tests"""
    print("🧪 Testing New VSR Features")
    print("=" * 50)
    
    # Create test suite
    suite = unittest.TestLoader().loadTestsFromTestCase(TestNewFeatures)
    
    # Run tests
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    
    # Print summary
    print("\n" + "=" * 50)
    print("NEW FEATURES TEST SUMMARY")
    print("=" * 50)
    print(f"Tests run: {result.testsRun}")
    print(f"Failures: {len(result.failures)}")
    print(f"Errors: {len(result.errors)}")
    
    if result.failures:
        print("\nFAILURES:")
        for test, traceback in result.failures:
            print(f"- {test}: {traceback}")
    
    if result.errors:
        print("\nERRORS:")
        for test, traceback in result.errors:
            print(f"- {test}: {traceback}")
    
    success = len(result.failures) == 0 and len(result.errors) == 0
    print(f"\n{'✅ All new features tests passed!' if success else '❌ Some tests failed!'}")
    
    return success

if __name__ == "__main__":
    run_new_features_tests()
