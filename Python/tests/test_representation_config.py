#!/usr/bin/env python3
"""
Unit tests for representation configuration functionality.
"""

import unittest
import tempfile
import json
import os
import shutil
from pathlib import Path
import sys

# Add parent directory to path to import vsr
sys.path.insert(0, str(Path(__file__).parent.parent))

from vsr import RepresentationConfig, VSRApp


class TestRepresentationConfig(unittest.TestCase):
    """Test cases for RepresentationConfig class."""
    
    def setUp(self):
        """Set up test fixtures."""
        self.temp_dir = tempfile.mkdtemp()
        self.config_dir = Path(self.temp_dir) / "test_configs"
        self.rep_config = RepresentationConfig(str(self.config_dir))
        
        # Create test data file
        self.test_data = {
            "users": {"alice": 100, "bob": 200, "charlie": 150},
            "products": [
                {"name": "laptop", "price": 1000},
                {"name": "mouse", "price": 25},
                {"name": "keyboard", "price": 75}
            ]
        }
        
        self.test_file = Path(self.temp_dir) / "test_data.json"
        with open(self.test_file, 'w') as f:
            json.dump(self.test_data, f)
    
    def tearDown(self):
        """Clean up test fixtures."""
        shutil.rmtree(self.temp_dir)
    
    def test_config_directory_creation(self):
        """Test that config directory is created."""
        self.assertTrue(self.config_dir.exists())
        self.assertTrue(self.config_dir.is_dir())
    
    def test_file_hash_generation(self):
        """Test file hash generation."""
        hash1 = self.rep_config._get_file_hash(str(self.test_file))
        hash2 = self.rep_config._get_file_hash(str(self.test_file))
        
        # Same file should produce same hash
        self.assertEqual(hash1, hash2)
        self.assertIsInstance(hash1, str)
        self.assertEqual(len(hash1), 32)  # MD5 hash length
    
    def test_file_hash_changes_with_content(self):
        """Test that hash changes when file content changes."""
        hash1 = self.rep_config._get_file_hash(str(self.test_file))
        
        # Modify file content
        modified_data = self.test_data.copy()
        modified_data["new_field"] = "new_value"
        
        with open(self.test_file, 'w') as f:
            json.dump(modified_data, f)
        
        hash2 = self.rep_config._get_file_hash(str(self.test_file))
        
        # Hash should be different
        self.assertNotEqual(hash1, hash2)
    
    def test_config_path_generation(self):
        """Test config path generation."""
        config_path = self.rep_config._get_config_path(str(self.test_file))
        
        self.assertIsInstance(config_path, Path)
        self.assertEqual(config_path.parent, self.config_dir)
        self.assertTrue(config_path.name.endswith('.json'))
    
    def test_save_and_load_config(self):
        """Test saving and loading configuration."""
        test_config = {
            "users": "table",
            "products": "bars"
        }
        
        # Save config
        self.rep_config.save_config(str(self.test_file), test_config)
        
        # Load config
        loaded_config = self.rep_config.load_config(str(self.test_file))
        
        self.assertIsNotNone(loaded_config)
        self.assertIn("config", loaded_config)
        self.assertEqual(loaded_config["config"], test_config)
        self.assertIn("file_path", loaded_config)
        self.assertIn("file_name", loaded_config)
        self.assertIn("created_at", loaded_config)
    
    def test_load_nonexistent_config(self):
        """Test loading configuration that doesn't exist."""
        nonexistent_file = Path(self.temp_dir) / "nonexistent.json"
        config = self.rep_config.load_config(str(nonexistent_file))
        
        self.assertIsNone(config)
    
    def test_delete_config(self):
        """Test deleting configuration."""
        test_config = {"users": "table"}
        
        # Save config first
        self.rep_config.save_config(str(self.test_file), test_config)
        
        # Verify it exists
        self.assertIsNotNone(self.rep_config.load_config(str(self.test_file)))
        
        # Delete config
        result = self.rep_config.delete_config(str(self.test_file))
        
        self.assertTrue(result)
        self.assertIsNone(self.rep_config.load_config(str(self.test_file)))
    
    def test_delete_nonexistent_config(self):
        """Test deleting configuration that doesn't exist."""
        nonexistent_file = Path(self.temp_dir) / "nonexistent.json"
        result = self.rep_config.delete_config(str(nonexistent_file))
        
        self.assertFalse(result)


class TestVSRAppConfiguration(unittest.TestCase):
    """Test cases for VSRApp configuration integration."""
    
    def setUp(self):
        """Set up test fixtures."""
        self.temp_dir = tempfile.mkdtemp()
        
        # Create test data file
        self.test_data = {
            "website_stats": {
                "visitors": 1000,
                "page_views": 5000,
                "bounce_rate": 0.3
            },
            "user_types": [
                {"type": "new", "count": 300},
                {"type": "returning", "count": 700}
            ]
        }
        
        self.test_file = Path(self.temp_dir) / "test_data.json"
        with open(self.test_file, 'w') as f:
            json.dump(self.test_data, f)
    
    def tearDown(self):
        """Clean up test fixtures."""
        shutil.rmtree(self.temp_dir)
    
    def test_identify_data_sets(self):
        """Test identification of data sets."""
        app = VSRApp(str(self.test_file))
        app.data = self.test_data
        
        data_sets = app.identify_data_sets(self.test_data)
        
        self.assertIn("website_stats", data_sets)
        self.assertIn("user_types", data_sets)
        self.assertEqual(len(data_sets), 2)
    
    def test_identify_data_sets_list_input(self):
        """Test identification of data sets with list input."""
        list_data = [
            {"name": "item1", "value": 10},
            {"name": "item2", "value": 20}
        ]
        
        app = VSRApp(str(self.test_file))
        data_sets = app.identify_data_sets(list_data)
        
        self.assertIn("data", data_sets)
        self.assertEqual(len(data_sets), 1)
        self.assertEqual(data_sets["data"]["data"], list_data)
        self.assertEqual(data_sets["data"]["type"], "list")
        self.assertEqual(data_sets["data"]["size"], 2)
    
    def test_identify_data_sets_empty_input(self):
        """Test identification of data sets with empty input."""
        app = VSRApp(str(self.test_file))
        
        # Test empty dict
        data_sets = app.identify_data_sets({})
        self.assertEqual(len(data_sets), 0)
        
        # Test empty list
        data_sets = app.identify_data_sets([])
        self.assertEqual(len(data_sets), 0)
    
    def test_config_integration(self):
        """Test that VSRApp properly integrates with RepresentationConfig."""
        app = VSRApp(str(self.test_file))
        
        # Check that rep_config is initialized
        self.assertIsNotNone(app.rep_config)
        self.assertIsInstance(app.rep_config, RepresentationConfig)
        
        # Check that config directory exists
        config_dir = Path("rep_saved")
        self.assertTrue(config_dir.exists())


class TestConfigurationWorkflow(unittest.TestCase):
    """Test cases for the complete configuration workflow."""
    
    def setUp(self):
        """Set up test fixtures."""
        self.temp_dir = tempfile.mkdtemp()
        self.config_dir = Path(self.temp_dir) / "test_configs"
        
        # Create test data file
        self.test_data = {
            "sales": {"q1": 1000, "q2": 1200, "q3": 1100, "q4": 1300},
            "expenses": {"rent": 500, "utilities": 200, "supplies": 100}
        }
        
        self.test_file = Path(self.temp_dir) / "business_data.json"
        with open(self.test_file, 'w') as f:
            json.dump(self.test_data, f)
    
    def tearDown(self):
        """Clean up test fixtures."""
        shutil.rmtree(self.temp_dir)
    
    def test_config_persistence(self):
        """Test that configuration persists across app instances."""
        # Create first app instance and save config
        rep_config = RepresentationConfig(str(self.config_dir))
        test_preferences = {
            "sales": "bars",
            "expenses": "table"
        }
        rep_config.save_config(str(self.test_file), test_preferences)
        
        # Create second app instance and load config
        rep_config2 = RepresentationConfig(str(self.config_dir))
        loaded_config = rep_config2.load_config(str(self.test_file))
        
        self.assertIsNotNone(loaded_config)
        self.assertEqual(loaded_config["config"], test_preferences)
    
    def test_config_file_structure(self):
        """Test the structure of saved configuration files."""
        rep_config = RepresentationConfig(str(self.config_dir))
        test_preferences = {"sales": "bars"}
        
        rep_config.save_config(str(self.test_file), test_preferences)
        
        # Load raw config file
        config_path = rep_config._get_config_path(str(self.test_file))
        with open(config_path, 'r') as f:
            raw_config = json.load(f)
        
        # Check required fields
        self.assertIn("file_path", raw_config)
        self.assertIn("file_name", raw_config)
        self.assertIn("created_at", raw_config)
        self.assertIn("config", raw_config)
        
        # Check values
        self.assertEqual(raw_config["file_name"], "business_data.json")
        self.assertEqual(raw_config["config"], test_preferences)
        self.assertIn("business_data.json", raw_config["file_path"])


class TestFileListingAndSelection(unittest.TestCase):
    """Test cases for file listing and selection functionality."""
    
    def setUp(self):
        """Set up test fixtures."""
        self.temp_dir = tempfile.mkdtemp()
        self.config_dir = Path(self.temp_dir) / "test_configs"
        self.rep_config = RepresentationConfig(str(self.config_dir))
        
        # Create multiple test data files
        self.test_files = []
        for i in range(3):
            test_data = {f"data_set_{i}": {f"item_{j}": j * 10 for j in range(3)}}
            test_file = Path(self.temp_dir) / f"test_data_{i}.json"
            with open(test_file, 'w') as f:
                json.dump(test_data, f)
            self.test_files.append(test_file)
        
        # Create configurations for these files
        for i, test_file in enumerate(self.test_files):
            config = {f"data_set_{i}": "table" if i % 2 == 0 else "bars"}
            self.rep_config.save_config(str(test_file), config)
    
    def tearDown(self):
        """Clean up test fixtures."""
        shutil.rmtree(self.temp_dir)
    
    def test_list_all_configs(self):
        """Test listing all configuration files."""
        configs = self.rep_config.list_all_configs()
        
        self.assertEqual(len(configs), 3)
        
        # Check that all configs have required fields
        for config in configs:
            self.assertIn("config_file", config)
            self.assertIn("file_path", config)
            self.assertIn("file_name", config)
            self.assertIn("created_at", config)
            self.assertIn("config", config)
            self.assertIn("file_exists", config)
            self.assertIn("file_size", config)
            
            # File should exist
            self.assertTrue(config["file_exists"])
            self.assertGreater(config["file_size"], 0)
    
    def test_list_configs_with_missing_files(self):
        """Test listing configs when some files are missing."""
        # Delete one of the test files
        self.test_files[1].unlink()
        
        configs = self.rep_config.list_all_configs()
        
        self.assertEqual(len(configs), 3)
        
        # Check that missing file is marked as not existing
        missing_file_config = None
        for config in configs:
            if config["file_name"] == "test_data_1.json":
                missing_file_config = config
                break
        
        self.assertIsNotNone(missing_file_config)
        self.assertFalse(missing_file_config["file_exists"])
        self.assertEqual(missing_file_config["file_size"], 0)
    
    def test_cleanup_missing_files(self):
        """Test cleanup of configurations for missing files."""
        # Delete two test files
        self.test_files[0].unlink()
        self.test_files[2].unlink()
        
        # Cleanup should remove 2 configurations
        removed_count = self.rep_config.cleanup_missing_files()
        self.assertEqual(removed_count, 2)
        
        # Only one config should remain
        configs = self.rep_config.list_all_configs()
        self.assertEqual(len(configs), 1)
        self.assertEqual(configs[0]["file_name"], "test_data_1.json")
    
    def test_file_size_calculation(self):
        """Test file size calculation."""
        test_file = self.test_files[0]
        file_size = self.rep_config._get_file_size(str(test_file))
        
        # File should have some size
        self.assertGreater(file_size, 0)
        
        # Test with non-existent file
        nonexistent_size = self.rep_config._get_file_size("nonexistent.json")
        self.assertEqual(nonexistent_size, 0)
    
    def test_config_sorting(self):
        """Test that configurations are sorted by creation date."""
        configs = self.rep_config.list_all_configs()
        
        # Should be sorted by creation date (newest first)
        creation_dates = [config["created_at"] for config in configs]
        sorted_dates = sorted(creation_dates, reverse=True)
        self.assertEqual(creation_dates, sorted_dates)
    
    def test_empty_config_directory(self):
        """Test behavior with empty config directory."""
        empty_config_dir = Path(self.temp_dir) / "empty_configs"
        empty_rep_config = RepresentationConfig(str(empty_config_dir))
        
        configs = empty_rep_config.list_all_configs()
        self.assertEqual(len(configs), 0)
        
        # Cleanup should return 0
        removed_count = empty_rep_config.cleanup_missing_files()
        self.assertEqual(removed_count, 0)
    
    def test_corrupted_config_files(self):
        """Test handling of corrupted configuration files."""
        # Create a corrupted config file
        corrupted_file = self.config_dir / "corrupted.json"
        with open(corrupted_file, 'w') as f:
            f.write("invalid json content {")
        
        # Should not crash and should skip corrupted files
        configs = self.rep_config.list_all_configs()
        self.assertEqual(len(configs), 3)  # Only valid configs


if __name__ == '__main__':
    unittest.main()
