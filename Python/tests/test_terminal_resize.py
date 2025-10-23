#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Test terminal resize detection functionality
"""

import sys
import os
import unittest
from unittest.mock import Mock, patch, MagicMock
import signal

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import vsr


class TestTerminalResizeDetection(unittest.TestCase):
    """Test suite for terminal resize detection."""

    def setUp(self):
        """Set up test fixtures."""
        # Create a test JSON file
        self.test_file = "test_resize_data.json"
        test_data = {
            "users": [
                {"name": "Alice", "age": 30},
                {"name": "Bob", "age": 25}
            ]
        }

        import json
        with open(self.test_file, 'w') as f:
            json.dump(test_data, f)

    def tearDown(self):
        """Clean up test files."""
        if os.path.exists(self.test_file):
            os.remove(self.test_file)

        # Clean up config files
        import shutil
        config_dir = os.path.join(os.path.dirname(__file__), '..', 'rep_saved')
        if os.path.exists(config_dir):
            try:
                shutil.rmtree(config_dir)
            except:
                pass

    def test_resize_handler_setup(self):
        """Test that resize handler is properly set up."""
        app = vsr.VSRApp(self.test_file)

        # Check that resize tracking flag exists
        self.assertFalse(app.terminal_size_changed)

        # On Unix systems, check that signal handler was registered
        if sys.platform != "win32":
            # The signal should be registered
            self.assertIsNotNone(signal.getsignal(signal.SIGWINCH))

    def test_resize_signal_handler_unix(self):
        """Test Unix signal handler for terminal resize."""
        if sys.platform == "win32":
            self.skipTest("Unix-only test")

        app = vsr.VSRApp(self.test_file)

        # Initially no resize
        self.assertFalse(app.terminal_size_changed)

        # Simulate resize signal
        app._handle_resize_signal(signal.SIGWINCH, None)

        # Flag should be set
        self.assertTrue(app.terminal_size_changed)

    def test_check_and_handle_resize_no_change(self):
        """Test resize check when terminal size hasn't changed."""
        app = vsr.VSRApp(self.test_file)

        # Set initial size
        app.terminal_width = 80
        app.terminal_height = 24

        # Mock get_terminal_size to return same size
        with patch.object(app, 'get_terminal_size', return_value=(80, 24)):
            result = app._check_and_handle_resize()

        # Should return False (no resize)
        self.assertFalse(result)

    def test_check_and_handle_resize_with_change(self):
        """Test resize check when terminal size has changed."""
        app = vsr.VSRApp(self.test_file)

        # Set initial size
        app.terminal_width = 80
        app.terminal_height = 24
        app.max_display_rows = 15

        # Trigger resize flag
        app.terminal_size_changed = True

        # Mock get_terminal_size to return new size
        with patch.object(app, 'get_terminal_size', return_value=(100, 30)):
            result = app._check_and_handle_resize()

        # Should return True (resize detected)
        self.assertTrue(result)

        # Terminal dimensions should be updated
        self.assertEqual(app.terminal_width, 100)
        self.assertEqual(app.terminal_height, 30)
        self.assertEqual(app.max_display_rows, 21)  # 30 - 9

        # Flag should be reset
        self.assertFalse(app.terminal_size_changed)

    def test_check_and_handle_resize_windows_polling(self):
        """Test Windows resize detection via polling."""
        if sys.platform != "win32":
            # Simulate Windows behavior
            original_platform = sys.platform
            sys.platform = "win32"

        try:
            app = vsr.VSRApp(self.test_file)

            # Set initial size
            app.terminal_width = 80
            app.terminal_height = 24

            # Mock get_terminal_size to return new size
            with patch.object(app, 'get_terminal_size', return_value=(120, 40)):
                result = app._check_and_handle_resize()

            # Should detect resize via polling
            self.assertTrue(result)
            self.assertEqual(app.terminal_width, 120)
            self.assertEqual(app.terminal_height, 40)

        finally:
            if 'original_platform' in locals():
                sys.platform = original_platform

    def test_resize_updates_max_display_rows(self):
        """Test that resize properly updates max_display_rows."""
        app = vsr.VSRApp(self.test_file)

        test_cases = [
            (80, 24, 15),   # 24 - 9 = 15
            (100, 30, 21),  # 30 - 9 = 21
            (120, 50, 41),  # 50 - 9 = 41
            (60, 15, 6),    # 15 - 9 = 6
        ]

        for width, height, expected_rows in test_cases:
            app.terminal_width = 0  # Force change
            app.terminal_height = 0
            app.terminal_size_changed = True

            with patch.object(app, 'get_terminal_size', return_value=(width, height)):
                app._check_and_handle_resize()

            self.assertEqual(app.max_display_rows, expected_rows,
                f"Expected {expected_rows} rows for {width}x{height}, got {app.max_display_rows}")

    def test_resize_flag_reset_after_handling(self):
        """Test that resize flag is properly reset after handling."""
        app = vsr.VSRApp(self.test_file)

        # Set resize flag
        app.terminal_size_changed = True

        # Mock terminal size change
        with patch.object(app, 'get_terminal_size', return_value=(100, 30)):
            app._check_and_handle_resize()

        # Flag should be reset
        self.assertFalse(app.terminal_size_changed)

        # Calling again without change should return False
        with patch.object(app, 'get_terminal_size', return_value=(100, 30)):
            result = app._check_and_handle_resize()

        self.assertFalse(result)


def run_tests():
    """Run all resize detection tests."""
    loader = unittest.TestLoader()
    suite = loader.loadTestsFromTestCase(TestTerminalResizeDetection)
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    return result.wasSuccessful()


if __name__ == '__main__':
    success = run_tests()
    sys.exit(0 if success else 1)
