#!/usr/bin/env python3
import sys
import os

# Enable UTF-8 output on Windows
if sys.platform == "win32":
    os.system("")
    sys.stdout.reconfigure(encoding='utf-8')

# Version information
__version__ = "0.9.0"

"""
VSR - A minimalistic terminal data visualizer
Version: {}
Usage: python vsr.py <file.json|file.csv>
""".format(__version__)

import sys
import json
import csv
import argparse
import os
import shutil
import hashlib
from pathlib import Path
from typing import Dict, List, Any, Optional, Tuple


class RepresentationConfig:
    """Manages representation configuration for data files."""
    
    def __init__(self, config_dir: str = "rep_saved"):
        self.config_dir = Path(config_dir)
        self.config_dir.mkdir(exist_ok=True)
    
    def _get_file_hash(self, filepath: str) -> str:
        """Generate hash based on file path, name, and content."""
        file_path = Path(filepath)
        
        # Read file content
        try:
            with open(file_path, 'rb') as f:
                content = f.read()
        except Exception:
            content = b""
        
        # Create hash from path, name, and content
        hash_input = f"{file_path.absolute()}:{file_path.name}:".encode() + content
        return hashlib.md5(hash_input).hexdigest()
    
    def _get_config_path(self, filepath: str) -> Path:
        """Get configuration file path for a data file."""
        file_hash = self._get_file_hash(filepath)
        return self.config_dir / f"{file_hash}.json"
    
    def load_config(self, filepath: str) -> Optional[Dict[str, Any]]:
        """Load representation configuration for a file."""
        config_path = self._get_config_path(filepath)
        
        if not config_path.exists():
            return None
        
        try:
            with open(config_path, 'r', encoding='utf-8') as f:
                return json.load(f)
        except Exception:
            return None
    
    def save_config(self, filepath: str, config: Dict[str, Any]):
        """Save representation configuration for a file."""
        config_path = self._get_config_path(filepath)
        
        # Add metadata
        config_data = {
            "file_path": str(Path(filepath).absolute()),
            "file_name": Path(filepath).name,
            "created_at": __import__('datetime').datetime.now().isoformat(),
            "config": config
        }
        
        try:
            with open(config_path, 'w', encoding='utf-8') as f:
                json.dump(config_data, f, indent=2)
        except Exception as e:
            print(f"Warning: Could not save config: {e}")
    
    def delete_config(self, filepath: str) -> bool:
        """Delete representation configuration for a file."""
        config_path = self._get_config_path(filepath)
        
        if config_path.exists():
            try:
                config_path.unlink()
                return True
            except Exception:
                return False
        return False
    
    def list_all_configs(self) -> List[Dict[str, Any]]:
        """List all saved configuration files with their metadata."""
        configs = []
        
        if not self.config_dir.exists():
            return configs
        
        for config_file in self.config_dir.glob("*.json"):
            try:
                with open(config_file, 'r', encoding='utf-8') as f:
                    config_data = json.load(f)
                
                # Check if file still exists
                file_path = config_data.get("file_path", "")
                file_exists = Path(file_path).exists() if file_path else False
                
                config_info = {
                    "config_file": str(config_file),
                    "file_path": file_path,
                    "file_name": config_data.get("file_name", "Unknown"),
                    "created_at": config_data.get("created_at", "Unknown"),
                    "config": config_data.get("config", {}),
                    "file_exists": file_exists,
                    "file_size": self._get_file_size(file_path) if file_exists else 0
                }
                configs.append(config_info)
                
            except Exception as e:
                # Skip corrupted config files
                continue
        
        # Sort by creation date (newest first)
        configs.sort(key=lambda x: x.get("created_at", ""), reverse=True)
        return configs
    
    def _get_file_size(self, filepath: str) -> int:
        """Get file size in bytes."""
        try:
            return Path(filepath).stat().st_size
        except Exception:
            return 0
    
    def cleanup_missing_files(self) -> int:
        """Remove configurations for files that no longer exist."""
        configs = self.list_all_configs()
        removed_count = 0
        
        for config in configs:
            if not config["file_exists"]:
                try:
                    Path(config["config_file"]).unlink()
                    removed_count += 1
                except Exception:
                    continue
        
        return removed_count


class VSRApp:
    """Main VSR application with ASCII-only interface."""
    
    def __init__(self, filename: str):
        self.filename = filename
        self.data = None
        self.processed_data = []
        self.data_sets = {}  # Store multiple data sets with their configs
        self.view_mode = "table"  # "table" or "bars"
        self.scroll_offset = 0
        self.terminal_width = 80
        self.terminal_height = 24
        self.max_display_rows = 20
        self.rep_config = RepresentationConfig()
        self.current_config = None
        # Slides functionality
        self.slides = {}  # Dictionary of slide_number -> list of data_set_names
        self.current_slide = 1
        self.total_slides = 1
        self.data_set_preferences = {}  # Store preferences with slide info
        
    def get_terminal_size(self) -> Tuple[int, int]:
        """Get current terminal size."""
        try:
            size = shutil.get_terminal_size()
            return size.columns, size.lines
        except:
            return 80, 24
    
    def clear_screen(self):
        """Clear the terminal screen."""
        os.system('cls' if os.name == 'nt' else 'clear')
    
    def load_data(self) -> Dict[str, Any]:
        """Load data from JSON or CSV file."""
        file_path = Path(self.filename)
        
        if not file_path.exists():
            raise FileNotFoundError(f"File not found: {self.filename}")
        
        if file_path.suffix.lower() == '.json':
            with open(file_path, 'r', encoding='utf-8') as f:
                return json.load(f)
        elif file_path.suffix.lower() == '.csv':
            with open(file_path, 'r', encoding='utf-8') as f:
                reader = csv.DictReader(f)
                return {"data": list(reader)}
        else:
            raise ValueError(f"Unsupported file format: {file_path.suffix}")
    
    def process_data(self, data: Dict[str, Any]) -> List[Dict[str, Any]]:
        """Process data into a uniform format for visualization."""
        processed = []
        
        if isinstance(data, dict):
            # Handle nested dictionary (like flat_data.json)
            for key, value in data.items():
                if isinstance(value, dict):
                    for sub_key, sub_value in value.items():
                        processed.append({"name": sub_key, "value": sub_value})
                elif isinstance(value, list):
                    for item in value:
                        if isinstance(item, dict):
                            processed.append(item)
                        else:
                            processed.append({"name": str(len(processed)), "value": item})
                else:
                    processed.append({"name": key, "value": value})
        elif isinstance(data, list):
            # Handle list of dictionaries
            processed = data
            
        return processed
    
    def identify_data_sets(self, data: Dict[str, Any]) -> Dict[str, Any]:
        """Identify different data sets within the loaded data."""
        data_sets = {}
        
        if isinstance(data, dict):
            for key, value in data.items():
                if isinstance(value, dict) and len(value) > 0:
                    # Dictionary data set (like "number_of_visitors")
                    data_sets[key] = {
                        'data': value,
                        'type': 'dict',
                        'size': len(value),
                        'sample_keys': list(value.keys()),  # Get all keys for column selection
                        'numeric_fields': self._get_numeric_fields_from_dict(value)
                    }
                elif isinstance(value, list) and len(value) > 0:
                    # List data set (like "user_types")
                    numeric_fields = self._get_numeric_fields_from_list(value)
                    # Get all available columns from the first item
                    all_keys = []
                    if value and isinstance(value[0], dict):
                        all_keys_set = set()
                        for item in value:
                            if isinstance(item, dict):
                                all_keys_set.update(item.keys())
                        all_keys = [key for key in all_keys_set if not key.startswith('_')]
                    
                    data_sets[key] = {
                        'data': value,
                        'type': 'list',
                        'size': len(value),
                        'sample_keys': all_keys,  # Get all keys for column selection
                        'numeric_fields': numeric_fields
                    }
        elif isinstance(data, list) and len(data) > 0:
            # Single data set
            numeric_fields = self._get_numeric_fields_from_list(data)
            # Get all available columns
            all_keys = []
            if data and isinstance(data[0], dict):
                all_keys_set = set()
                for item in data:
                    if isinstance(item, dict):
                        all_keys_set.update(item.keys())
                all_keys = [key for key in all_keys_set if not key.startswith('_')]
            
            data_sets["data"] = {
                'data': data,
                'type': 'list',
                'size': len(data),
                'sample_keys': all_keys,  # Get all keys for column selection
                'numeric_fields': numeric_fields
            }
        
        return data_sets
    
    def _get_numeric_fields_from_dict(self, data: Dict[str, Any]) -> List[str]:
        """Get list of numeric field names from a dictionary."""
        numeric_fields = []
        for key, value in data.items():
            if isinstance(value, (int, float)):
                numeric_fields.append(key)
        return numeric_fields
    
    def _get_numeric_fields_from_list(self, data: List[Any]) -> List[str]:
        """Get list of numeric field names from a list of dictionaries."""
        numeric_fields = []
        if not data or not isinstance(data[0], dict):
            return numeric_fields
        
        # Check first item to identify numeric fields
        first_item = data[0]
        for key, value in first_item.items():
            if isinstance(value, (int, float)):
                numeric_fields.append(key)
        
        return numeric_fields
    
    def process_multiple_data_sets(self, data_sets: Dict[str, Any], preferences: Dict[str, Any]) -> List[Dict[str, Any]]:
        """Process multiple data sets based on their individual configurations."""
        all_processed_data = []
        
        # Only process data sets that are in preferences (not skipped)
        for set_name, set_info in data_sets.items():
            if set_name not in preferences:
                continue  # Skip data sets that are not in preferences
            
            set_config = preferences[set_name]
            set_data = set_info['data']
            
            # Process this data set
            if set_info['type'] == 'dict':
                # Dictionary data set (like "number_of_visitors")
                processed_set = []
                for key, value in set_data.items():
                    processed_set.append({
                        "name": key,
                        "value": value,
                        "_data_set": set_name,
                        "_config": set_config
                    })
            elif set_info['type'] == 'list':
                # List data set (like "user_types")
                processed_set = []
                for item in set_data:
                    if isinstance(item, dict):
                        # Add metadata to each item
                        processed_item = item.copy()
                        processed_item["_data_set"] = set_name
                        processed_item["_config"] = set_config
                        processed_set.append(processed_item)
                    else:
                        processed_set.append({
                            "name": str(item),
                            "value": item,
                            "_data_set": set_name,
                            "_config": set_config
                        })
            else:
                processed_set = []
            
            all_processed_data.extend(processed_set)
        
        return all_processed_data
    
    def ask_representation_preferences(self, data_sets: Dict[str, Any]) -> Dict[str, Any]:
        """Ask user how they want to represent each data set with page-by-page navigation."""
        preferences = {}
        data_set_list = list(data_sets.items())
        current_index = 0
        
        while current_index < len(data_set_list):
            set_name, set_info = data_set_list[current_index]
            
            # Configure current data set
            data_set_names = [name for name, _ in data_set_list]
            config_result = self._configure_single_data_set(
                set_name, set_info, current_index, len(data_set_list), preferences, data_set_names
            )
            
            if config_result == 'next':
                # If navigating to next without configuring, mark current as skipped
                if set_name not in preferences:
                    preferences[set_name] = {'type': 'skip'}
                current_index += 1
            elif config_result == 'previous':
                # If navigating to previous without configuring, mark current as skipped
                if set_name not in preferences:
                    preferences[set_name] = {'type': 'skip'}
                current_index = max(0, current_index - 1)
            elif config_result == 'quit':
                # If quitting without configuring current data set, mark as skipped
                if set_name not in preferences:
                    preferences[set_name] = {'type': 'skip'}
                # Return partial preferences if user quits
                return preferences
            elif isinstance(config_result, dict):
                # Configuration completed for this data set
                preferences[set_name] = config_result
                current_index += 1
        
        # Show final summary
        self._show_configuration_summary(preferences)
        
        # Filter out skipped data sets from final preferences
        filtered_preferences = {name: config for name, config in preferences.items() 
                               if config.get('type') != 'skip'}
        
        return filtered_preferences
    
    def _configure_single_data_set(self, set_name: str, set_info: Dict[str, Any], 
                                   current_index: int, total_count: int, 
                                   existing_preferences: Dict[str, Any], 
                                   data_set_names: List[str]) -> Any:
        """Configure a single data set with navigation options."""
        while True:
            self.clear_screen()
            print(f"🔧 Configuration Setup - Data Set {current_index + 1}/{total_count}")
            print("=" * 60)
            print(f"File: {self.filename}")
            print(f"Configuring: {set_name}\n")
            
            # Show data set information
            print(f"📊 Data Set Details:")
            print(f"   Type: {set_info['type'].title()} with {set_info['size']} items")
            if set_info['sample_keys']:
                print(f"   Sample keys: {', '.join(set_info['sample_keys'])}")
            if set_info['numeric_fields']:
                print(f"   Numeric fields: {', '.join(set_info['numeric_fields'])}")
            print()
            
            # Show existing configuration if any
            if set_name in existing_preferences:
                config = existing_preferences[set_name]
                print(f"✓ Current configuration: {config['type'].title()}")
                if 'field' in config and config['field']:
                    print(f"   Field: {config['field']}")
                if 'columns' in config:
                    print(f"   Columns: {', '.join(config['columns'])}")
                print()
            
            # Show progress indicator
            progress_bar = self._create_progress_bar(current_index, total_count, existing_preferences, data_set_names)
            print(f"Progress: {progress_bar}\n")
            
            # Show options
            print("Choose representation:")
            print("[t] Table view - Shows data in tabular format")
            print("[b] Bar chart - Shows data as horizontal bars")
            print("[r] Tree view - Shows nested data hierarchically")
            print("[s] Skip - Omit this data set from display")
            print()
            print("Slide options:")
            print("[1-9] Put on slide number (1-9)")
            print("[n] Create new slide for this data set")
            print()
            
            # Navigation options
            nav_options = []
            if current_index > 0:
                nav_options.append("[←] Previous data set (skip current)")
            if current_index < total_count - 1:
                nav_options.append("[→] Next data set (skip current)")
            nav_options.append("[q] Quit configuration")
            
            if nav_options:
                print("Navigation:")
                for option in nav_options:
                    print(f"  {option}")
                print("Use arrow keys ← → for navigation (skips current data set)")
                print()
            
            # Get user input
            print(f"Choice for '{set_name}': ", end='', flush=True)
            choice = self._get_key_input()
            
            # Handle representation selection first
            config = None
            if choice == 't':
                # Configure table view
                config = {'type': 'table'}
                
                # Get all available columns (excluding metadata)
                all_columns = [key for key in set_info['sample_keys'] if not key.startswith('_')]
                
                # If more than 2 columns, ask user to select which ones to display
                if len(all_columns) > 2:
                    selected_columns = self._ask_column_selection(set_name, all_columns)
                    if selected_columns:  # Check if user didn't quit
                        config['columns'] = selected_columns
                    else:
                        continue  # Go back to main configuration if user quit column selection
                        
            elif choice == 'b':
                # Configure bar chart
                bar_field = None
                numeric_fields = set_info['numeric_fields']
                
                if len(numeric_fields) > 1:
                    # Use visual selection for multiple fields
                    bar_field = self._ask_bar_field_selection(set_name, numeric_fields)
                    if not bar_field:  # Check if user didn't quit
                        continue  # Go back to main configuration if user quit field selection
                elif len(numeric_fields) == 1:
                    bar_field = numeric_fields[0]
                else:
                    # No numeric fields, use default logic (will be handled in visualization)
                    bar_field = None
                
                config = {
                    'type': 'bars',
                    'field': bar_field
                }
                
            elif choice == 'r':
                # Configure tree view for nested data
                config = {'type': 'tree'}
                
            elif choice == 's':
                # Skip this data set - return special skip configuration
                config = {'type': 'skip'}
                
            elif choice == 'left' and current_index > 0:
                return 'previous'
            elif choice == 'right' and current_index < total_count - 1:
                return 'next'
            elif choice == 'q':
                return 'quit'
            elif choice.isdigit() and 1 <= int(choice) <= 9:
                # User selected a slide number - ask for representation first
                print(f"\n📄 Selected slide {choice}. Now choose representation type:")
                print("[t] Table [b] Bars [r] Tree [s] Skip")
                print("Representation: ", end='', flush=True)
                rep_choice = self._get_key_input()
                
                if rep_choice == 't':
                    config = {'type': 'table'}
                    all_columns = [key for key in set_info['sample_keys'] if not key.startswith('_')]
                    if len(all_columns) > 2:
                        selected_columns = self._ask_column_selection(set_name, all_columns)
                        if selected_columns:
                            config['columns'] = selected_columns
                        else:
                            continue
                elif rep_choice == 'b':
                    bar_field = None
                    numeric_fields = set_info['numeric_fields']
                    if len(numeric_fields) > 1:
                        bar_field = self._ask_bar_field_selection(set_name, numeric_fields)
                        if not bar_field:
                            continue
                    elif len(numeric_fields) == 1:
                        bar_field = numeric_fields[0]
                    config = {'type': 'bars', 'field': bar_field}
                elif rep_choice == 'r':
                    config = {'type': 'tree'}
                elif rep_choice == 's':
                    config = {'type': 'skip'}
                else:
                    print(f"\n❌ Invalid representation choice '{rep_choice}'.")
                    import time
                    time.sleep(2)
                    continue
                    
                # Add slide information to config
                if config:
                    config['slide'] = int(choice)
                    return config
                    
            elif choice == 'n':
                # Create new slide - ask for representation first
                new_slide = max(existing_preferences.get(name, {}).get('slide', 1) for name in existing_preferences.keys()) + 1 if existing_preferences else 1
                print(f"\n📄 Creating new slide {new_slide}. Now choose representation type:")
                print("[t] Table [b] Bars [r] Tree [s] Skip")
                print("Representation: ", end='', flush=True)
                rep_choice = self._get_key_input()
                
                if rep_choice == 't':
                    config = {'type': 'table'}
                    all_columns = [key for key in set_info['sample_keys'] if not key.startswith('_')]
                    if len(all_columns) > 2:
                        selected_columns = self._ask_column_selection(set_name, all_columns)
                        if selected_columns:
                            config['columns'] = selected_columns
                        else:
                            continue
                elif rep_choice == 'b':
                    bar_field = None
                    numeric_fields = set_info['numeric_fields']
                    if len(numeric_fields) > 1:
                        bar_field = self._ask_bar_field_selection(set_name, numeric_fields)
                        if not bar_field:
                            continue
                    elif len(numeric_fields) == 1:
                        bar_field = numeric_fields[0]
                    config = {'type': 'bars', 'field': bar_field}
                elif rep_choice == 'r':
                    config = {'type': 'tree'}
                elif rep_choice == 's':
                    config = {'type': 'skip'}
                else:
                    print(f"\n❌ Invalid representation choice '{rep_choice}'.")
                    import time
                    time.sleep(2)
                    continue
                    
                # Add slide information to config
                if config:
                    config['slide'] = new_slide
                    return config
            else:
                # Invalid input - show error and continue loop
                print(f"\n❌ Invalid choice '{choice}'. Press 't' for table, 'b' for bars, 'r' for tree, 's' to skip, 1-9 for slide number, 'n' for new slide, ← → for navigation, or 'q' to quit.")
                import time
                time.sleep(2)
                continue
            
            # If we have a config but no slide specified, ask for slide
            if config and config.get('type') != 'skip':
                slide_num = self._ask_slide_selection(set_name, existing_preferences)
                if slide_num:
                    config['slide'] = slide_num
                    return config
                else:
                    continue  # Go back if user quit slide selection
    
    def _create_progress_bar(self, current_index: int, total_count: int, 
                           existing_preferences: Dict[str, Any], 
                           data_set_names: List[str]) -> str:
        """Create a visual progress bar showing configuration status."""
        progress_chars = []
        
        for i in range(total_count):
            if i < current_index:
                # Check if this data set was skipped
                set_name = data_set_names[i]
                if set_name in existing_preferences and existing_preferences[set_name].get('type') == 'skip':
                    progress_chars.append("➖")  # Skipped (heavy minus sign)
                else:
                    progress_chars.append("✓")  # Completed
            elif i == current_index:
                progress_chars.append("●")  # Current
            else:
                progress_chars.append("○")  # Pending
        
        # Count configured (non-skipped) data sets
        configured_count = sum(1 for name in data_set_names[:current_index] 
                              if name in existing_preferences and existing_preferences[name].get('type') != 'skip')
        skipped_count = sum(1 for name in data_set_names[:current_index] 
                           if name in existing_preferences and existing_preferences[name].get('type') == 'skip')
        
        status = f" ({current_index + 1}/{total_count})"
        if configured_count > 0 or skipped_count > 0:
            status += f" [Configured: {configured_count}, Skipped: {skipped_count}]"
        
        return " ".join(progress_chars) + status
    
    def _show_configuration_summary(self, preferences: Dict[str, Any]) -> None:
        """Show a summary of all configurations before proceeding."""
        self.clear_screen()
        print("🎉 Configuration Complete!")
        print("=" * 50)
        print(f"File: {self.filename}")
        
        # Separate configured and skipped data sets
        configured = {name: config for name, config in preferences.items() if config.get('type') != 'skip'}
        skipped = {name: config for name, config in preferences.items() if config.get('type') == 'skip'}
        
        print(f"Total data sets: {len(preferences)}")
        print(f"Configured for display: {len(configured)}")
        print(f"Skipped: {len(skipped)}\n")
        
        if configured:
            # Group by slides
            slides_data = {}
            for set_name, config in configured.items():
                slide_num = config.get('slide', 1)
                if slide_num not in slides_data:
                    slides_data[slide_num] = []
                slides_data[slide_num].append((set_name, config))
            
            print("📊 Data sets organized by slides:")
            for slide_num in sorted(slides_data.keys()):
                print(f"\n📄 Slide {slide_num}:")
                for set_name, config in slides_data[slide_num]:
                    print(f"  • {set_name}")
                    print(f"    View: {config['type'].title()}")
                    if 'field' in config and config['field']:
                        print(f"    Field: {config['field']}")
                    if 'columns' in config:
                        print(f"    Columns: {', '.join(config['columns'])}")
            print()
        
        if skipped:
            print("➖ Skipped data sets:")
            for set_name in skipped.keys():
                print(f"  - {set_name}")
            print()
        
        if not configured:
            print("⚠️  No data sets configured for display!")
            print("All data sets were skipped. Nothing will be visualized.\n")
        
        print("Press any key to continue...")
        self._get_key_input()
    
    def _ask_column_selection(self, set_name: str, columns: List[str]) -> List[str]:
        """Ask user to select columns for table view using arrow keys and space bar."""
        selected = [False] * len(columns)  # Track selected columns
        current_index = 0  # Current cursor position
        
        while True:
            self.clear_screen()
            print(f"📋 Select columns to display for '{set_name}'")
            print("=" * 50)
            print("Use ↑/↓ arrow keys to navigate, SPACE to select/deselect, ENTER to confirm")
            print("At least 2 columns must be selected\n")
            
            # Display columns with selection status
            for i, column in enumerate(columns):
                cursor = "→ " if i == current_index else "  "
                checkbox = "☑" if selected[i] else "☐"
                print(f"{cursor}{checkbox} {column}")
            
            # Show current selection count
            selected_count = sum(selected)
            print(f"\nSelected: {selected_count}/{len(columns)} columns")
            
            if selected_count >= 2:
                print("✓ Ready to confirm (press ENTER)")
            else:
                print("❌ Select at least 2 columns")
            
            # Get user input
            key = self._get_key_input()
            
            if key == 'up' and current_index > 0:
                current_index -= 1
            elif key == 'down' and current_index < len(columns) - 1:
                current_index += 1
            elif key == 'space':
                selected[current_index] = not selected[current_index]
            elif key == 'enter':
                if selected_count >= 2:
                    # Return selected columns in original order
                    selected_columns = [columns[i] for i in range(len(columns)) if selected[i]]
                    print(f"\n✓ Selected columns: {', '.join(selected_columns)}")
                    import time
                    time.sleep(1)  # Brief confirmation display
                    return selected_columns
                else:
                    # Flash error message
                    print("\n❌ At least 2 columns must be selected!")
                    import time
                    time.sleep(1)
            elif key == 'a':  # 'a' for select all
                selected = [True] * len(columns)
            elif key == 'n':  # 'n' for select none
                selected = [False] * len(columns)
            elif key == 'q':  # Allow quit
                return None  # Return None to indicate user quit
    
    def _ask_bar_field_selection(self, set_name: str, numeric_fields: List[str]) -> str:
        """Ask user to select a single field for bar chart using arrow keys and space bar."""
        selected_index = 0  # Track selected field (only one allowed)
        current_index = 0  # Current cursor position
        
        while True:
            self.clear_screen()
            print(f"📊 Select field for bar chart for '{set_name}'")
            print("=" * 50)
            print("Use ↑/↓ arrow keys to navigate, SPACE to select, ENTER to confirm")
            print("Select exactly one numeric field\n")
            
            # Display fields with selection status
            for i, field in enumerate(numeric_fields):
                cursor = "→ " if i == current_index else "  "
                checkbox = "☑" if i == selected_index else "☐"
                print(f"{cursor}{checkbox} {field}")
            
            # Show current selection
            print(f"\nSelected field: {numeric_fields[selected_index]}")
            print("✓ Ready to confirm (press ENTER)")
            
            # Get user input
            key = self._get_key_input()
            
            if key == 'up' and current_index > 0:
                current_index -= 1
            elif key == 'down' and current_index < len(numeric_fields) - 1:
                current_index += 1
            elif key == 'space':
                selected_index = current_index  # Only one selection allowed
            elif key == 'enter':
                selected_field = numeric_fields[selected_index]
                print(f"\n✓ Selected field: {selected_field}")
                import time
                time.sleep(1)  # Brief confirmation display
                return selected_field
            elif key == 'q':  # Allow quit
                return None  # Return None to indicate user quit
    
    def _ask_slide_selection(self, set_name: str, existing_preferences: Dict[str, Any]) -> int:
        """Ask user to select which slide to put the data set on."""
        # Get existing slides from preferences
        existing_slides = set()
        for prefs in existing_preferences.values():
            if isinstance(prefs, dict) and 'slide' in prefs:
                existing_slides.add(prefs['slide'])
        
        # Create list of available slides (1-9)
        available_slides = list(range(1, 10))
        
        while True:
            self.clear_screen()
            print(f"📄 Select slide for '{set_name}'")
            print("=" * 50)
            print("Choose which slide to place this data set on:\n")
            
            # Show existing slides
            if existing_slides:
                print("Existing slides:")
                for slide_num in sorted(existing_slides):
                    datasets_on_slide = [name for name, prefs in existing_preferences.items() 
                                        if isinstance(prefs, dict) and prefs.get('slide') == slide_num]
                    print(f"  Slide {slide_num}: {', '.join(datasets_on_slide)}")
                print()
            
            print("Options:")
            print("[1-9] Put on slide number (1-9)")
            print("[n] Create new slide")
            print("[q] Quit slide selection")
            print()
            
            print(f"Slide choice for '{set_name}': ", end='', flush=True)
            choice = self._get_key_input()
            
            if choice.isdigit() and 1 <= int(choice) <= 9:
                slide_num = int(choice)
                print(f"\n✓ Selected slide {slide_num}")
                import time
                time.sleep(1)
                return slide_num
            elif choice == 'n':
                # Create new slide
                new_slide = max(existing_slides) + 1 if existing_slides else 1
                if new_slide <= 9:
                    print(f"\n✓ Created new slide {new_slide}")
                    import time
                    time.sleep(1)
                    return new_slide
                else:
                    print("\n❌ Maximum 9 slides allowed!")
                    import time
                    time.sleep(2)
            elif choice == 'q':
                return None
            else:
                print(f"\n❌ Invalid choice '{choice}'. Press 1-9 for slide number, 'n' for new slide, or 'q' to quit.")
                import time
                time.sleep(2)
    
    def _get_key_input(self) -> str:
        """Get keyboard input and return normalized key name."""
        try:
            if os.name == 'nt':  # Windows
                import msvcrt
                key = msvcrt.getch()
                
                # Handle special keys (arrows, etc.)
                if key == b'\xe0':  # Arrow key prefix on Windows
                    key = msvcrt.getch()
                    if key == b'H':  # Up arrow
                        return 'up'
                    elif key == b'P':  # Down arrow
                        return 'down'
                    elif key == b'K':  # Left arrow
                        return 'left'
                    elif key == b'M':  # Right arrow
                        return 'right'
                elif key == b'\r':  # Enter
                    return 'enter'
                elif key == b' ':  # Space
                    return 'space'
                else:
                    try:
                        return key.decode('utf-8').lower()
                    except UnicodeDecodeError:
                        return 'unknown'
            else:  # Unix/Linux/Mac
                import termios, tty
                fd = sys.stdin.fileno()
                old_settings = termios.tcgetattr(fd)
                try:
                    tty.setraw(sys.stdin.fileno())
                    key = sys.stdin.read(1)
                    
                    # Handle escape sequences (arrow keys)
                    if key == '\x1b':  # ESC sequence
                        key += sys.stdin.read(2)
                        if key == '\x1b[A':  # Up arrow
                            return 'up'
                        elif key == '\x1b[B':  # Down arrow
                            return 'down'
                        elif key == '\x1b[C':  # Right arrow
                            return 'right'
                        elif key == '\x1b[D':  # Left arrow
                            return 'left'
                    elif key == '\r' or key == '\n':  # Enter
                        return 'enter'
                    elif key == ' ':  # Space
                        return 'space'
                    else:
                        return key.lower()
                finally:
                    termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
        except Exception:
            return 'unknown'
        
        return 'unknown'
    
    def load_or_create_config(self, data: Dict[str, Any]) -> Dict[str, Any]:
        """Load existing config or create new one by asking user."""
        # Try to load existing config
        config_data = self.rep_config.load_config(self.filename)
        
        if config_data and "config" in config_data:
            self.current_config = config_data
            return config_data["config"]
        
        # No config exists, ask user
        data_sets = self.identify_data_sets(data)
        preferences = self.ask_representation_preferences(data_sets)
        
        # Save the new config
        self.rep_config.save_config(self.filename, preferences)
        
        return preferences
    
    def reconfigure_representations(self):
        """Allow user to reconfigure representation preferences."""
        self.clear_screen()
        print("🔧 Reconfiguring Representations")
        print("=" * 50)
        
        # Delete existing config
        if self.rep_config.delete_config(self.filename):
            print("✓ Previous configuration deleted")
        else:
            print("ℹ No previous configuration found")
        
        print("\nPress any key to continue...")
        if os.name == 'nt':
            import msvcrt
            msvcrt.getch()
        else:
            input()
        
        # Ask for new preferences
        data_sets = self.identify_data_sets(self.data)
        preferences = self.ask_representation_preferences(data_sets)
        
        # Save new config
        self.rep_config.save_config(self.filename, preferences)
        
        # Update current view based on first data set
        first_set = list(preferences.keys())[0]
        self.view_mode = preferences[first_set]['type']
        
        # Store preferences for use in visualization
        self.data_set_preferences = preferences
        
        # Reprocess data with new configuration
        self.processed_data = self.process_multiple_data_sets(self.data_sets, preferences)
        
        # Reset scroll position
        self.scroll_offset = 0
        
        print("\n✓ Configuration saved!")
        print("Press any key to continue...")
        if os.name == 'nt':
            import msvcrt
            msvcrt.getch()
        else:
            input()
    
    def show_file_selection_menu(self) -> Optional[str]:
        """Show file selection menu with arrow key navigation and return selected file path."""
        configs = self.rep_config.list_all_configs()
        
        if not configs:
            self.clear_screen()
            print("📁 No Previous Files Found")
            print("=" * 50)
            print("No representation configurations found.")
            print("Please run VSR with a file path to get started:")
            print("\nUsage: python vsr.py <file.json|file.csv>")
            print("\nExample:")
            print("  python vsr.py examples/data.json")
            print("\nPress any key to exit...")
            if os.name == 'nt':
                import msvcrt
                msvcrt.getch()
            else:
                input()
            return None
        
        # Filter only valid (existing) files
        valid_files = []
        for config in configs:
            if config["file_exists"]:
                valid_files.append(config)
        
        if not valid_files:
            # No valid files, show cleanup options
            while True:
                self.clear_screen()
                print("📁 Select Data File to Visualize")
                print("=" * 50)
                print("❌ No valid files found. All configured files are missing.")
                print("\nOptions:")
                print("  [c] - Clean up missing files")
                print("  [q] - Quit")
                print("\nChoice: ", end="")
                
                choice = input().lower().strip()
                if choice == 'c':
                    removed = self.rep_config.cleanup_missing_files()
                    self.clear_screen()
                    print(f"🗑️  Removed {removed} configuration(s) for missing files.")
                    print("Press any key to continue...")
                    if os.name == 'nt':
                        import msvcrt
                        msvcrt.getch()
                    else:
                        input()
                    # Refresh configs after cleanup
                    configs = self.rep_config.list_all_configs()
                    valid_files = [config for config in configs if config["file_exists"]]
                    if valid_files:
                        break  # Exit cleanup loop and proceed to file selection
                    continue
                elif choice == 'q':
                    return None
                else:
                    continue
        
        # Arrow key navigation for file selection
        selected_index = 0
        
        while True:
            self.clear_screen()
            print("📁 Select Data File to Visualize")
            print("=" * 50)
            print("Use ↑/↓ arrow keys to navigate, ENTER to select, 'q' to quit")
            print(f"Found {len(valid_files)} valid file(s):\n")
            
            # Display file list with cursor
            for i, config in enumerate(valid_files):
                file_path = config["file_path"]
                file_name = config["file_name"]
                file_size = config["file_size"]
                created_at = config["created_at"]
                
                # Format file size
                if file_size > 1024 * 1024:
                    size_str = f"{file_size / (1024 * 1024):.1f}MB"
                elif file_size > 1024:
                    size_str = f"{file_size / 1024:.1f}KB"
                else:
                    size_str = f"{file_size}B"
                
                # Format creation date
                try:
                    from datetime import datetime
                    dt = datetime.fromisoformat(created_at.replace('Z', '+00:00'))
                    date_str = dt.strftime("%Y-%m-%d %H:%M")
                except:
                    date_str = "Unknown"
                
                # Show cursor for selected item
                cursor = "→" if i == selected_index else " "
                print(f"{cursor} {file_path}")
                print()
            
            print("\nOptions:")
            print("  [↑/↓] - Navigate files")
            print("  [ENTER] - Select file")
            print("  [c] - Clean up missing files")
            print("  [r] - Refresh list")
            print("  [q] - Quit")
            
            # Get keyboard input
            key = self._get_key_input()
            
            if key == 'up':
                selected_index = (selected_index - 1) % len(valid_files)
            elif key == 'down':
                selected_index = (selected_index + 1) % len(valid_files)
            elif key == 'enter':
                # Return selected file path
                return valid_files[selected_index]["file_path"]
            elif key == 'q':
                return None
            elif key == 'c':
                removed = self.rep_config.cleanup_missing_files()
                self.clear_screen()
                print(f"🗑️  Removed {removed} configuration(s) for missing files.")
                print("Press any key to continue...")
                if os.name == 'nt':
                    import msvcrt
                    msvcrt.getch()
                else:
                    input()
                # Refresh configs after cleanup
                configs = self.rep_config.list_all_configs()
                valid_files = [config for config in configs if config["file_exists"]]
                if not valid_files:
                    return None  # No files left after cleanup
                selected_index = min(selected_index, len(valid_files) - 1)
            elif key == 'r':
                # Refresh list
                configs = self.rep_config.list_all_configs()
                valid_files = [config for config in configs if config["file_exists"]]
                if not valid_files:
                    return None  # No files left after refresh
                selected_index = min(selected_index, len(valid_files) - 1)
    
    def create_table_view(self) -> str:
        """Create ASCII table view for multiple data sets."""
        if not self.processed_data:
            return "No data to display"
        
        lines = []
        current_line_count = 0
        max_lines = self.max_display_rows
        
        # Group data by data set
        data_by_set = {}
        for item in self.processed_data:
            set_name = item.get('_data_set', 'data')
            if set_name not in data_by_set:
                data_by_set[set_name] = []
            data_by_set[set_name].append(item)
        
        # Process each data set
        for set_name, set_data in data_by_set.items():
            if current_line_count >= max_lines:
                break
                
            # Get configuration for this data set
            set_config = set_data[0].get('_config', {'type': 'table'}) if set_data else {'type': 'table'}
            
            # Only show tables for data sets configured as tables
            if set_config.get('type') != 'table':
                continue
            
            # Add data set header
            if len(data_by_set) > 1:  # Only show header if multiple data sets
                lines.append(f"📊 {set_name.replace('_', ' ').title()}")
                lines.append("─" * min(len(set_name) + 4, self.terminal_width))
                current_line_count += 2
            
            # Get columns for this data set (exclude metadata)
            all_keys = set()
            for item in set_data:
                for key in item.keys():
                    if not key.startswith('_'):
                        all_keys.add(key)
            
            if not all_keys:
                continue
                
            columns = sorted(list(all_keys))
            
            # Calculate column widths
            min_col_widths = {}
            for col in columns:
                max_width = len(col)  # Header width
                for item in set_data:
                    value_str = str(item.get(col, ""))
                    max_width = max(max_width, len(value_str))
                min_col_widths[col] = min(max_width + 2, 20)  # Limit column width
            
            # Calculate total minimum width needed
            min_table_width = sum(min_col_widths.values()) + len(columns) + 1
            
            # Expand columns if needed
            col_widths = min_col_widths.copy()
            if min_table_width < self.terminal_width:
                extra_space = self.terminal_width - min_table_width
                space_per_column = extra_space // len(columns)
                remaining_space = extra_space % len(columns)
                
                for i, col in enumerate(columns):
                    col_widths[col] += space_per_column
                    if i < remaining_space:
                        col_widths[col] += 1
            
            # Create table for this data set
            table_lines = []
            
            # Top border
            top_border = "┌"
            for i, col in enumerate(columns):
                top_border += "─" * col_widths[col]
                top_border += "┐" if i == len(columns) - 1 else "┬"
            table_lines.append(top_border)
            
            # Header row
            header_line = "│"
            for col in columns:
                header_text = f" {col.title():<{col_widths[col]-1}}"
                header_line += header_text + "│"
            table_lines.append(header_line)
            
            # Header separator
            separator = "├"
            for i, col in enumerate(columns):
                separator += "─" * col_widths[col]
                separator += "┤" if i == len(columns) - 1 else "┼"
            table_lines.append(separator)
            
            # Data rows with pagination
            available_rows = max_lines - current_line_count - len(table_lines) - 1  # -1 for bottom border
            if available_rows > 0:
                start_idx = max(0, self.scroll_offset)
                end_idx = min(start_idx + available_rows, len(set_data))
                
                for i in range(start_idx, end_idx):
                    if i < len(set_data):
                        item = set_data[i]
                        data_line = "│"
                        for col in columns:
                            value = str(item.get(col, ""))
                            if len(value) > col_widths[col] - 2:
                                value = value[:col_widths[col] - 5] + "..."
                            data_text = f" {value:<{col_widths[col]-1}}"
                            data_line += data_text + "│"
                        table_lines.append(data_line)
            
            # Bottom border
            bottom_border = "└"
            for i, col in enumerate(columns):
                bottom_border += "─" * col_widths[col]
                bottom_border += "┘" if i == len(columns) - 1 else "┴"
            table_lines.append(bottom_border)
            
            # Add table lines to output
            lines.extend(table_lines)
            current_line_count += len(table_lines)
            
            # Add spacing between data sets
            if len(data_by_set) > 1 and current_line_count < max_lines:
                lines.append("")
                current_line_count += 1
        
        return "\n".join(lines)
    
    def create_bar_view(self) -> str:
        """Create ASCII horizontal bar chart view for multiple data sets."""
        if not self.processed_data:
            return "No data to display"
        
        lines = []
        current_line_count = 0
        max_lines = self.max_display_rows
        
        # Group data by data set
        data_by_set = {}
        for item in self.processed_data:
            set_name = item.get('_data_set', 'data')
            if set_name not in data_by_set:
                data_by_set[set_name] = []
            data_by_set[set_name].append(item)
        
        # Process each data set
        for set_name, set_data in data_by_set.items():
            if current_line_count >= max_lines:
                break
                
            # Get configuration for this data set
            set_config = set_data[0].get('_config', {'type': 'bars'}) if set_data else {'type': 'bars'}
            
            # Only show bars for data sets configured as bars
            if set_config.get('type') != 'bars':
                continue
            
            # Add data set header
            if len(data_by_set) > 1:  # Only show header if multiple data sets
                lines.append(f"📊 {set_name.replace('_', ' ').title()}")
                lines.append("─" * min(len(set_name) + 4, self.terminal_width))
                current_line_count += 2
            
            # Extract numeric values for bar chart
            bar_data = []
            preferred_field = set_config.get('field')
            
            for item in set_data:
                name = str(item.get("name", ""))
                numeric_value = None
                
                # Use the preferred field if specified
                if preferred_field and preferred_field in item:
                    try:
                        numeric_value = float(item[preferred_field])
                    except (ValueError, TypeError):
                        pass
                
                # If no preferred field or it failed, try common fields
                if numeric_value is None:
                    for field in ["value", "population", "count", "amount", "size", "total"]:
                        if field in item and not field.startswith('_'):
                            try:
                                numeric_value = float(item[field])
                                break
                            except (ValueError, TypeError):
                                continue
                
                # If still no numeric field found, try all fields
                if numeric_value is None:
                    for key, value in item.items():
                        if not key.startswith('_') and key.lower() != "name":
                            try:
                                numeric_value = float(value)
                                break
                            except (ValueError, TypeError):
                                continue
                
                # If still no numeric value, use string length or 1
                if numeric_value is None:
                    value = item.get("value", item.get("name", ""))
                    if isinstance(value, str):
                        numeric_value = len(value)
                    else:
                        numeric_value = 1
                
                bar_data.append((name, numeric_value))
            
            if not bar_data:
                continue
            
            # Sort by value (descending)
            bar_data.sort(key=lambda x: x[1], reverse=True)
            
            # Calculate bars
            max_value = max(value for _, value in bar_data) if bar_data else 1
            bar_width = min(50, self.terminal_width - 30)  # Reserve space for labels and values
            
            # Add bars with pagination
            available_rows = max_lines - current_line_count
            if available_rows > 0:
                start_idx = max(0, self.scroll_offset)
                end_idx = min(start_idx + available_rows, len(bar_data))
                
                for i in range(start_idx, end_idx):
                    if i < len(bar_data):
                        name, value = bar_data[i]
                        
                        if max_value > 0:
                            filled_length = int((value / max_value) * bar_width)
                        else:
                            filled_length = 0
                        
                        empty_length = bar_width - filled_length
                        
                        # Use block characters for bars
                        filled_bars = "█" * filled_length
                        empty_bars = "▒" * empty_length
                        
                        # Format the line
                        name_part = f"{name:<15}"[:15]  # Truncate long names
                        bar_part = f"{filled_bars}{empty_bars}"
                        value_part = f"{value:>10}"
                        
                        line = f"{name_part} {bar_part} {value_part}"
                        lines.append(line)
                        current_line_count += 1
            
            # Add spacing between data sets
            if len(data_by_set) > 1 and current_line_count < max_lines:
                lines.append("")
                current_line_count += 1
        
        return "\n".join(lines)
    
    def create_tree_view_for_set(self, set_name: str, set_data: List[Dict[str, Any]], max_lines: int) -> str:
        """Create tree view for nested JSON data."""
        tree_lines = []
        
        if not set_data:
            return "No data"
        
        # For tree view, we show all items in the data set
        # Each item typically has 'name' and 'value' fields from the data processing
        for i, item in enumerate(set_data):
            if len(tree_lines) >= max_lines:
                break
                
            # Get the data item (removing metadata)
            data_item = {k: v for k, v in item.items() if not k.startswith('_')}
            
            # Render each item as a tree node
            is_last_item = i == len(set_data) - 1
            self._render_tree_node(data_item, tree_lines, "", is_last_item, max_lines)
            
            # Add spacing between items if there are multiple items and we're not at the last one
            if len(set_data) > 1 and not is_last_item and len(tree_lines) < max_lines:
                tree_lines.append("")
        
        return "\n".join(tree_lines[:max_lines])
    
    def _render_tree_node(self, node: Any, lines: List[str], prefix: str, is_last: bool, max_lines: int, key: str = None) -> None:
        """Recursively render a tree node with ASCII characters."""
        if len(lines) >= max_lines:
            return
        
        # Determine the connector
        if key is not None:
            connector = "└─ " if is_last else "├─ "
            line = prefix + connector + str(key) + ": "
        else:
            line = ""
        
        # Handle different node types
        if isinstance(node, dict):
            if key is not None:
                lines.append(line)
            # Get items for consistent ordering
            items = list(node.items())
            for i, (k, v) in enumerate(items):
                if len(lines) >= max_lines:
                    break
                is_last_item = i == len(items) - 1
                # Adjust prefix for children
                if key is not None:
                    child_prefix = prefix + ("    " if is_last else "│   ")
                else:
                    child_prefix = prefix
                self._render_tree_node(v, lines, child_prefix, is_last_item, max_lines, k)
        
        elif isinstance(node, list):
            lines.append(line + f"[{len(node)} items]")
            # Show first few items of the list
            for i, item in enumerate(node[:3]):  # Show only first 3 items
                if len(lines) >= max_lines:
                    break
                is_last_item = i == min(len(node), 3) - 1 and len(node) <= 3
                child_prefix = prefix + ("    " if is_last else "│   ")
                self._render_tree_node(item, lines, child_prefix, is_last_item, max_lines, f"[{i}]")
            
            # Add ellipsis if there are more items
            if len(node) > 3 and len(lines) < max_lines:
                child_prefix = prefix + ("    " if is_last else "│   ")
                lines.append(child_prefix + "└─ ...")
        
        else:
            # Leaf node - show the value
            value_str = str(node)
            # Truncate long values
            if len(value_str) > 50:
                value_str = value_str[:47] + "..."
            lines.append(line + value_str)
    
    def create_mixed_view(self) -> str:
        """Create mixed view showing both tables and bars based on data set configurations."""
        if not self.processed_data:
            return "No data to display"
        
        lines = []
        current_line_count = 0
        max_lines = self.max_display_rows
        
        # Group data by data set
        data_by_set = {}
        for item in self.processed_data:
            set_name = item.get('_data_set', 'data')
            if set_name not in data_by_set:
                data_by_set[set_name] = []
            data_by_set[set_name].append(item)
        
        # Process each data set according to its configuration
        # Note: We don't limit by max_lines here since scrolling is handled at the mixed view level
        for set_name, set_data in data_by_set.items():
            # Get configuration for this data set
            set_config = set_data[0].get('_config', {'type': 'table'}) if set_data else {'type': 'table'}
            view_type = set_config.get('type', 'table')
            
            # Add data set header if multiple data sets
            if len(data_by_set) > 1:
                lines.append(f"📊 {set_name.replace('_', ' ').title()} ({view_type})")
                lines.append("─" * min(len(set_name) + 10, self.terminal_width))
            
            # Create view based on configuration - allow full view since scrolling is handled later
            if view_type == 'table':
                set_view = self.create_table_view_for_set(set_name, set_data, 1000)  # Large number to allow full table
            elif view_type == 'bars':
                set_view = self.create_bar_view_for_set(set_name, set_data, set_config, 1000)  # Large number to allow full bars
            elif view_type == 'tree':
                set_view = self.create_tree_view_for_set(set_name, set_data, 1000)  # Large number to allow full tree
            else:
                set_view = "Invalid view type"
            
            if set_view:
                # Split multi-line views into individual lines
                set_lines = set_view.split('\n')
                lines.extend(set_lines)
            
            # Add spacing between data sets
            if len(data_by_set) > 1:
                lines.append("")
        
        # Apply global scroll offset to the final assembled view
        all_lines = lines  # lines is already a list of individual lines
        
        # Apply scroll offset
        if self.scroll_offset > 0 and self.scroll_offset < len(all_lines):
            visible_lines = all_lines[self.scroll_offset:self.scroll_offset + max_lines]
        else:
            visible_lines = all_lines[:max_lines]
        
        return "\n".join(visible_lines)
    
    def get_total_mixed_view_lines(self) -> int:
        """Get the total number of lines in the mixed view without scroll offset applied."""
        if not self.processed_data:
            return 0
        
        # Calculate total lines by generating the full mixed view
        lines = []
        
        # Group data by data set
        data_by_set = {}
        for item in self.processed_data:
            set_name = item.get('_data_set', 'data')
            if set_name not in data_by_set:
                data_by_set[set_name] = []
            data_by_set[set_name].append(item)
        
        # Process each data set to count total lines
        for set_name, set_data in data_by_set.items():
            # Get configuration for this data set
            set_config = set_data[0].get('_config', {'type': 'table'}) if set_data else {'type': 'table'}
            view_type = set_config.get('type', 'table')
            
            # Add data set header if multiple data sets
            if len(data_by_set) > 1:
                lines.append(f"📊 {set_name.replace('_', ' ').title()} ({view_type})")
                lines.append("─" * min(len(set_name) + 10, self.terminal_width))
            
            # Create view based on configuration
            if view_type == 'table':
                set_view = self.create_table_view_for_set(set_name, set_data, 10000)  # Large number to get all lines
            elif view_type == 'bars':
                set_view = self.create_bar_view_for_set(set_name, set_data, set_config, 10000)
            elif view_type == 'tree':
                set_view = self.create_tree_view_for_set(set_name, set_data, 10000)
            else:
                set_view = "Invalid view type"
            
            if set_view:
                # Split multi-line views into individual lines
                set_lines = set_view.split('\n')
                lines.extend(set_lines)
            
            # Add spacing between data sets
            if len(data_by_set) > 1:
                lines.append("")
        
        return len(lines)
    
    def create_table_view_for_set(self, set_name: str, set_data: List[Dict[str, Any]], max_lines: int) -> str:
        """Create table view for a specific data set."""
        if not set_data:
            return ""
        
        # Get configuration for this data set
        set_config = set_data[0].get('_config', {}) if set_data else {}
        
        # Get columns for this data set (exclude metadata)
        all_keys = set()
        for item in set_data:
            for key in item.keys():
                if not key.startswith('_'):
                    all_keys.add(key)
        
        if not all_keys:
            return ""
        
        # Use selected columns from configuration if available, otherwise use all columns
        if 'columns' in set_config and set_config['columns']:
            # Filter to only include columns that exist in the data
            configured_columns = [col for col in set_config['columns'] if col in all_keys]
            columns = configured_columns if configured_columns else sorted(list(all_keys))
        else:
            columns = sorted(list(all_keys))
        
        # Calculate column widths
        min_col_widths = {}
        for col in columns:
            max_width = len(col)  # Header width
            for item in set_data:
                value_str = str(item.get(col, ""))
                max_width = max(max_width, len(value_str))
            min_col_widths[col] = min(max_width + 2, 20)  # Limit column width
        
        # Calculate total minimum width needed
        min_table_width = sum(min_col_widths.values()) + len(columns) + 1
        
        # Expand columns if needed
        col_widths = min_col_widths.copy()
        if min_table_width < self.terminal_width:
            extra_space = self.terminal_width - min_table_width
            space_per_column = extra_space // len(columns)
            remaining_space = extra_space % len(columns)
            
            for i, col in enumerate(columns):
                col_widths[col] += space_per_column
                if i < remaining_space:
                    col_widths[col] += 1
        
        # Create table
        table_lines = []
        
        # Top border
        top_border = "┌"
        for i, col in enumerate(columns):
            top_border += "─" * col_widths[col]
            top_border += "┐" if i == len(columns) - 1 else "┬"
        table_lines.append(top_border)
        
        # Header row
        header_line = "│"
        for col in columns:
            header_text = f" {col.title():<{col_widths[col]-1}}"
            header_line += header_text + "│"
        table_lines.append(header_line)
        
        # Header separator
        separator = "├"
        for i, col in enumerate(columns):
            separator += "─" * col_widths[col]
            separator += "┤" if i == len(columns) - 1 else "┼"
        table_lines.append(separator)
        
        # Data rows - show all data for this set (scrolling handled at mixed view level)
        available_rows = max_lines - len(table_lines) - 1  # -1 for bottom border
        if available_rows > 0:
            # Show data from the beginning of this data set, up to available rows
            end_idx = min(available_rows, len(set_data))
            
            for i in range(end_idx):
                if i < len(set_data):
                    item = set_data[i]
                    data_line = "│"
                    for col in columns:
                        value = str(item.get(col, ""))
                        if len(value) > col_widths[col] - 2:
                            value = value[:col_widths[col] - 5] + "..."
                        data_text = f" {value:<{col_widths[col]-1}}"
                        data_line += data_text + "│"
                    table_lines.append(data_line)
        
        # Bottom border
        bottom_border = "└"
        for i, col in enumerate(columns):
            bottom_border += "─" * col_widths[col]
            bottom_border += "┘" if i == len(columns) - 1 else "┴"
        table_lines.append(bottom_border)
        
        return "\n".join(table_lines)
    
    def create_bar_view_for_set(self, set_name: str, set_data: List[Dict[str, Any]], set_config: Dict[str, Any], max_lines: int) -> str:
        """Create bar view for a specific data set."""
        if not set_data:
            return ""
        
        # Extract numeric values for bar chart
        bar_data = []
        preferred_field = set_config.get('field')
        
        for item in set_data:
            name = str(item.get("name", ""))
            numeric_value = None
            
            # Use the preferred field if specified
            if preferred_field and preferred_field in item:
                try:
                    numeric_value = float(item[preferred_field])
                except (ValueError, TypeError):
                    pass
            
            # If no preferred field or it failed, try common fields
            if numeric_value is None:
                for field in ["value", "population", "count", "amount", "size", "total"]:
                    if field in item and not field.startswith('_'):
                        try:
                            numeric_value = float(item[field])
                            break
                        except (ValueError, TypeError):
                            continue
            
            # If still no numeric field found, try all fields
            if numeric_value is None:
                for key, value in item.items():
                    if not key.startswith('_') and key.lower() != "name":
                        try:
                            numeric_value = float(value)
                            break
                        except (ValueError, TypeError):
                            continue
            
            # If still no numeric value, use string length or 1
            if numeric_value is None:
                value = item.get("value", item.get("name", ""))
                if isinstance(value, str):
                    numeric_value = len(value)
                else:
                    numeric_value = 1
            
            bar_data.append((name, numeric_value))
        
        if not bar_data:
            return ""
        
        # Sort by value (descending)
        bar_data.sort(key=lambda x: x[1], reverse=True)
        
        # Calculate bars
        max_value = max(value for _, value in bar_data) if bar_data else 1
        bar_width = min(50, self.terminal_width - 30)  # Reserve space for labels and values
        
        # Create bars - show all data for this set (scrolling handled at mixed view level)
        bar_lines = []
        if max_lines > 0:
            # Show data from the beginning of this data set, up to max_lines
            end_idx = min(max_lines, len(bar_data))
            
            for i in range(end_idx):
                if i < len(bar_data):
                    name, value = bar_data[i]
                    
                    if max_value > 0:
                        filled_length = int((value / max_value) * bar_width)
                    else:
                        filled_length = 0
                    
                    empty_length = bar_width - filled_length
                    
                    # Use block characters for bars
                    filled_bars = "█" * filled_length
                    empty_bars = "▒" * empty_length
                    
                    # Format the line
                    name_part = f"{name:<15}"[:15]  # Truncate long names
                    bar_part = f"{filled_bars}{empty_bars}"
                    value_part = f"{value:>10}"
                    
                    line = f"{name_part} {bar_part} {value_part}"
                    bar_lines.append(line)
        
        return "\n".join(bar_lines)
    
    def show_help(self) -> str:
        """Show help information."""
        help_text = f"""
VSR - Terminal Data Visualizer (ASCII Version)
Version: {__version__}

Key Commands:
  j - Scroll down
  k - Scroll up
  g - Go to top
  G - Go to bottom
  ← - Previous slide (when multiple slides exist)
  → - Next slide (when multiple slides exist)
  r - Refresh screen / check for resize
  c - Configure representations (override saved config)
  h - Show this help
  q - Quit

Navigation:
  Use j/k for vim-like scrolling
  Use g/G to jump to top/bottom
  Use ←/→ to navigate between slides
  Use r or Ctrl+L to refresh the display
  Use c to reconfigure how data sets are displayed

View Configuration:
  View modes (table/bars) are set during initial configuration
  Use 'c' to reconfigure representation preferences
  
Data Set Configuration Options:
  t - Configure as table view
  b - Configure as bar chart view
  r - Configure as tree view (for nested data)
  s - Skip data set (omit from display)
  1-9 - Put data set on slide number (1-9)
  n - Create new slide for data set
  ← - Go to previous data set
  → - Go to next data set
  q - Quit configuration
  
Column Selection (during configuration):
  ↑/↓ - Navigate between columns
  SPACE - Select/deselect column
  ENTER - Confirm selection (minimum 2 columns)
  a - Select all columns
  n - Select none (clear all)
  q - Quit column selection
  
Bar Field Selection (during configuration):
  ↑/↓ - Navigate between numeric fields
  SPACE - Select field (only one allowed)
  ENTER - Confirm selection
  q - Quit field selection
  
Skip Functionality:
  Press 's' during configuration to skip a data set
  Skipped data sets are omitted from visualization
  Progress bar shows ➖ for skipped data sets
  Configuration summary separates configured and skipped data sets
  
Slide/Page Functionality:
  Organize data sets into separate slides/pages
  During configuration, specify which slide each data set should go on
  Use 1-9 to put data set on specific slide number
  Use 'n' to create a new slide for the data set
  During viewing, use ←/→ to navigate between slides
  Each slide can contain multiple data sets with different visualization types
  Slide information shown in header: "Slide X/Y (Z data sets)"
  
Press any key to continue...
        """
        return help_text.strip()
    
    def display_screen(self):
        """Display the current screen."""
        self.terminal_width, self.terminal_height = self.get_terminal_size()
        
        # Reserve space: header(3) + empty(1) + footer(3) = 9 lines
        self.max_display_rows = self.terminal_height - 9
        
        self.clear_screen()
        
        # Header - show slide info if slides are configured
        if hasattr(self, 'slides') and self.total_slides > 1:
            # Count data sets on current slide
            current_slide_datasets = len(self.slides.get(self.current_slide, []))
            header = f"VSR - {self.filename} | Slide {self.current_slide}/{self.total_slides} ({current_slide_datasets} data sets)"
        elif hasattr(self, 'data_set_preferences') and len(self.data_set_preferences) > 1:
            header = f"VSR - {self.filename} | Mixed View ({len(self.data_set_preferences)} data sets)"
        else:
            header = f"VSR - {self.filename} | Mode: {self.view_mode.title()}"
        
        print("=" * self.terminal_width)
        print(f"{header:^{self.terminal_width}}")
        print("=" * self.terminal_width)
        print()
        
        # Content - create mixed view showing both tables and bars
        content = self.create_mixed_view()
        
        print(content)
        
        # Calculate remaining space and add padding
        content_lines = content.count('\n') + 1
        remaining_lines = self.max_display_rows - content_lines
        
        if remaining_lines > 0:
            print('\n' * remaining_lines)
        
        # Pagination info - show line numbers, not data item numbers
        total_lines = self.get_total_mixed_view_lines()
        start_line = self.scroll_offset + 1
        end_line = min(self.scroll_offset + self.max_display_rows, total_lines)
        
        if total_lines > 0:
            pagination = f"Showing lines {start_line}-{end_line} of {total_lines}"
        else:
            pagination = "No data"
        
        # Compressed footer: separator + showing + hotkeys
        print("-" * self.terminal_width)
        print(f"{pagination:^{self.terminal_width}}")
        
        # Hot keys row - include slide navigation if multiple slides
        if hasattr(self, 'slides') and self.total_slides > 1:
            hotkeys = "[j/k]Scroll [g/G]Top/Bottom [←/→]Slides [r]Refresh [c]Config [h]Help [q]Quit"
        else:
            hotkeys = "[j/k]Scroll [g/G]Top/Bottom [r]Refresh [c]Config [h]Help [q]Quit"
        print(f"{hotkeys:^{self.terminal_width}}")
    

    
    def handle_input(self, key: str) -> bool:
        """Handle user input. Returns False if should quit."""
        # Handle uppercase G before converting to lowercase
        is_uppercase_g = (key == 'G')
        key = key.lower().strip()
        
        if key == 'q':
            return False
        elif key == 'j':
            total_lines = self.get_total_mixed_view_lines()
            max_offset = max(0, total_lines - self.max_display_rows)
            self.scroll_offset = min(self.scroll_offset + 1, max_offset)
        elif key == 'k':
            self.scroll_offset = max(0, self.scroll_offset - 1)
        elif key == 'g' and not is_uppercase_g:
            self.scroll_offset = 0
        elif key == 'g' and is_uppercase_g:  # This was uppercase G
            total_lines = self.get_total_mixed_view_lines()
            max_offset = max(0, total_lines - self.max_display_rows)
            self.scroll_offset = max_offset
        elif key == 'h':
            self.clear_screen()
            print(self.show_help())
            input()  # Wait for any key
        elif key == 'r' or (len(key) == 1 and ord(key) == 12):  # 'r' key or Ctrl+L
            # Force refresh and resize check
            self.terminal_width, self.terminal_height = self.get_terminal_size()
            self.max_display_rows = self.terminal_height - 9
            self.clear_screen()
            print("🔄 Screen refreshed")
            import time
            time.sleep(0.3)
            self.clear_screen()
        elif key == 'c':
            # Reconfigure representations
            self.reconfigure_representations()
            self.clear_screen()
        elif key == 'left' and hasattr(self, 'slides') and self.total_slides > 1:
            # Navigate to previous slide
            self.current_slide = max(1, self.current_slide - 1)
            self.scroll_offset = 0  # Reset scroll when changing slides
            self._update_processed_data_for_current_slide()
        elif key == 'right' and hasattr(self, 'slides') and self.total_slides > 1:
            # Navigate to next slide
            self.current_slide = min(self.total_slides, self.current_slide + 1)
            self.scroll_offset = 0  # Reset scroll when changing slides
            self._update_processed_data_for_current_slide()
        
        return True
    
    def reconfigure_representations(self):
        """Allow user to reconfigure representation preferences."""
        # Delete existing config
        if self.current_config:
            self.rep_config.delete_config(self.filename)
        
        # Ask for new preferences
        preferences = self.ask_representation_preferences(self.data_sets)
        
        # Save new config
        self.rep_config.save_config(self.filename, preferences)
        
        # Update view mode based on first data set preference
        if preferences and self.data_sets:
            first_set = list(self.data_sets.keys())[0]
            first_set_config = preferences.get(first_set, {"type": "table"})
            self.view_mode = first_set_config.get("type", "table")
        
        # Store preferences for use in visualization
        self.data_set_preferences = preferences
        
        # Reorganize slides based on new preferences
        self._organize_slides_from_preferences(preferences)
        
        # Reprocess data with new configuration
        self._update_processed_data_for_current_slide()
        
        # Reset scroll position
        self.scroll_offset = 0
    
    def _update_processed_data_for_current_slide(self):
        """Update processed data to show only data sets from the current slide."""
        if not hasattr(self, 'slides') or self.current_slide not in self.slides:
            return
        
        # Get data sets for current slide
        current_slide_datasets = self.slides[self.current_slide]
        
        # Filter preferences to only include current slide's data sets
        current_slide_preferences = {name: config for name, config in self.data_set_preferences.items() 
                                   if name in current_slide_datasets}
        
        # Filter data sets to only include current slide's data sets
        current_slide_data_sets = {name: data for name, data in self.data_sets.items() 
                                 if name in current_slide_datasets}
        
        # Reprocess data for current slide
        self.processed_data = self.process_multiple_data_sets(current_slide_data_sets, current_slide_preferences)
    
    def _organize_slides_from_preferences(self, preferences: Dict[str, Any]):
        """Organize data sets into slides based on their slide configuration."""
        self.slides = {}
        self.total_slides = 1
        
        # Group data sets by slide number
        for set_name, config in preferences.items():
            slide_num = config.get('slide', 1)  # Default to slide 1 if not specified
            if slide_num not in self.slides:
                self.slides[slide_num] = []
            self.slides[slide_num].append(set_name)
            self.total_slides = max(self.total_slides, slide_num)
        
        # Ensure we have at least slide 1
        if not self.slides:
            self.slides[1] = []
        
        # If no slides were created (empty preferences), create slide 1 with all data sets
        if not preferences:
            self.slides[1] = list(self.data_sets.keys()) if hasattr(self, 'data_sets') else []
        
        # Set current slide to 1
        self.current_slide = 1
    
    def run(self):
        """Main application loop."""
        try:
            # Clear screen on startup
            self.clear_screen()
            
            # Load and process data
            self.data = self.load_data()
            
            # Load or create representation configuration
            preferences = self.load_or_create_config(self.data)
            
            # Identify data sets and process them
            self.data_sets = self.identify_data_sets(self.data)
            
            # Set initial view mode based on first data set preference
            if preferences and self.data_sets:
                first_set = list(self.data_sets.keys())[0]
                first_set_config = preferences.get(first_set, {"type": "table"})
                self.view_mode = first_set_config.get("type", "table")
            
            # Store preferences for use in visualization
            self.data_set_preferences = preferences
            
            # Organize data sets into slides
            self._organize_slides_from_preferences(preferences)
            
            # Process data sets for the current slide (initially slide 1)
            self._update_processed_data_for_current_slide()
            
            if not self.processed_data:
                print("No data found in file")
                return
            
            # Store initial terminal size
            current_width, current_height = self.get_terminal_size()
            
            # Display initial screen
            self.display_screen()
            
            # Main loop
            while True:
                try:
                    # Get user input using the proper method that handles arrow keys
                    key = self._get_key_input()
                    
                    # Check for terminal resize after user input
                    new_width, new_height = self.get_terminal_size()
                    if new_width != current_width or new_height != current_height:
                        current_width, current_height = new_width, new_height
                        # Terminal was resized, show message and redraw
                        self.clear_screen()
                        print(f"🔄 Terminal resized to {new_width}x{new_height}")
                        import time
                        time.sleep(0.5)  # Brief pause to show resize message
                        self.clear_screen()
                        self.display_screen()
                    
                    # Handle the user input
                    if not self.handle_input(key):
                        break
                    
                    # Redraw screen after handling input
                    self.display_screen()
                        
                except KeyboardInterrupt:
                    break
                except Exception as e:
                    print(f"Input error: {e}")
                    break
            
        except Exception as e:
            print(f"Error: {e}")
        finally:
            self.clear_screen()
            print("bb from vsr")


def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(description=f"VSR - Terminal Data Visualizer (ASCII) v{__version__}")
    parser.add_argument("file", nargs='?', help="JSON or CSV file to visualize (optional)")
    parser.add_argument("-v", "--version", action="version", version=f"VSR {__version__}")
    
    args = parser.parse_args()
    
    if args.file:
        # File provided as argument
        app = VSRApp(args.file)
        app.run()
    else:
        # No file provided, show file selection menu
        app = VSRApp("")
        app.clear_screen()  # Clear terminal before showing file selection menu
        selected_file = app.show_file_selection_menu()
        
        if selected_file:
            # User selected a file, create new app instance with the selected file
            app = VSRApp(selected_file)
            app.run()
        else:
            # User cancelled or no files available
            app.clear_screen()
            print("Goodbye!")


if __name__ == "__main__":
    main()
