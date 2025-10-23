# VSR - Terminal Data Visualizer

**Version: 0.9.2**

A comprehensive, vim-like terminal application for visualizing data from JSON and CSV files with advanced features including tree view, arrow key navigation, and intelligent configuration management.

## Features

### 🎯 Core Visualization
- **Multiple File Formats**: Supports JSON and CSV files with intelligent data detection
- **Triple Visualization Modes**: Professional table view, horizontal bar charts, and hierarchical tree view
- **Unicode Box-Drawing**: Beautiful table borders using Unicode characters (┌─┐│├┤└┘)
- **Full-Width & Responsive**: Views automatically expand and adapt to terminal width, even on resize
- **Mixed View Display**: Show different data sets with different visualization types in one view

### 🎮 Advanced Navigation
- **Arrow Key Navigation**: Modern, intuitive navigation for all selection screens (files, columns, fields)
- **File Selection Menu**: Interactive menu with arrow key navigation when running without arguments
- **Vim-like Key Bindings**: Familiar navigation (`j`/`k` for scrolling, `g`/`G` for top/bottom)
- **Smooth Scrolling**: Fixed scrolling bugs for large data sets with proper pagination
- **Cross-Platform Input**: Robust keyboard handling for Windows, macOS, and Linux

### ⚙️ Configuration System
- **Smart Representation Configs**: Automatically remembers how you prefer to view each data file
- **Page-by-Page Configuration**: Configure multiple data sets individually with forward/backward navigation
- **Skip Data Sets**: Choose to skip specific data sets during configuration to omit them from display
- **Manual Column Ordering**: Interactive column selection with manual ordering - columns appear in the order you select them
- **Visual Selection Feedback**: Shows numbered indicators (1), (2), (3) next to manually selected columns
- **Preserved CSV Column Order**: CSV files maintain their original column order from headers
- **Field Selection**: Smart field selection for bar charts with multiple numeric fields
- **Configuration Persistence**: Saves preferences with file metadata and automatic cleanup

### 🌳 Tree View (NEW in v0.8.0)
- **Hierarchical Display**: Visualize nested JSON structures with ASCII tree characters
- **Smart List Handling**: Summarizes long lists and shows first few items
- **Value Truncation**: Handles long values gracefully with ellipsis
- **Multi-Level Nesting**: Supports unlimited nesting depth with proper indentation
- **Mixed Integration**: Tree views work seamlessly with table and bar views

### 📄 Slides/Pages (NEW in v0.9.0)
- **Multi-Page Organization**: Organize data sets into separate slides/pages (up to 9 slides)
- **Slide Navigation**: Use ← → arrow keys to navigate between slides during viewing
- **Flexible Assignment**: Assign data sets to specific slides during configuration
- **Mixed Content**: Each slide can contain multiple data sets with different visualization types
- **Presentation Mode**: Create slide-based presentations of your data
- **Smart Interface**: Header shows current slide info, footer shows navigation options

### 🔄 Recent Improvements

**v0.9.2** (Latest)
- **Optimized Terminal Resize Detection**: Signal-based resize handling on Unix/Linux/macOS (zero overhead)
- **Instant Resize Response**: Automatic screen redraw when terminal is resized
- **Cross-Platform Efficiency**: Uses SIGWINCH signals on Unix, smart polling on Windows
- **Improved User Experience**: Clear resize notifications and seamless viewport adjustments

**v0.9.1**
- **Manual Column Selection Order**: Columns now appear in the exact order you select them manually
- **Visual Order Indicators**: Selected columns show numbered indicators (1), (2), (3) for clarity
- **CSV Column Order Preservation**: CSV files maintain their original header column order
- **Enhanced User Control**: Better control over table column arrangement and display
- **Order Status Messages**: Clear feedback about whether using manual or default column ordering

### 🔧 Technical Excellence
- **Zero Dependencies**: Uses only standard Python libraries
- **Cross-Platform**: Works on Windows, macOS, and Linux terminals
- **UTF-8 Support**: Proper Unicode character rendering on all platforms
- **Error Handling**: Graceful handling of missing files, corrupted data, and invalid input
- **Memory Efficient**: Optimized for large data sets with smart pagination

## Usage

### Direct File Visualization
```bash
python vsr.py <file.json|file.csv>
```

### Interactive File Selection
If you run the application without arguments, you get a file selection menu:
```bash
python vsr.py
```

```
📁 Select Data File to Visualize
==================================================
Use ↑/↓ arrow keys to navigate, ENTER to select, 'q' to quit
Found 3 valid file(s):

  /path/to/examples/comprehensive_data.json

→ /path/to/examples/nested_tree_data.json

  /path/to/examples/sample_data.csv


Options:
  [↑/↓] - Navigate files
  [ENTER] - Select file
  [c] - Clean up missing files
  [r] - Refresh list
  [q] - Quit
```

### Version Information
```bash
python vsr.py --version
python vsr.py -v
```

### Table View Visualization
Display data in professional table format with Unicode box-drawing characters:
```
📊 Employee Data
==================================================
┌─────────────────┬─────┬──────────────┬────────────┐
│ name            │ age │ department   │ salary     │
├─────────────────┼─────┼──────────────┼────────────┤
│ Alice Johnson   │  28 │ Engineering  │     75000  │
│ Bob Smith       │  35 │ Marketing    │     65000  │
│ Carol Davis     │  42 │ Engineering  │     85000  │
│ David Wilson    │  31 │ Sales        │     58000  │
│ Eva Brown       │  29 │ HR           │     62000  │
└─────────────────┴─────┴──────────────┴────────────┘
```

### Bar Chart Visualization
Visualize numeric data with horizontal ASCII bar charts:
```
📊 Sales Performance
==================================================
Alice Johnson   ████████████████████████████████████████ 125000
Bob Smith       ████████████████████████████████         95000
Carol Davis     ████████████████████████████████████████ 140000
David Wilson    ████████████████████████                 78000
Eva Brown       ██████████████████████████████████       110000
```

### Tree View Visualization
Visualize nested JSON structures with hierarchical tree display:
```
📊 Communication Metrics
==================================================
├─ telegram: 
│   ├─ sent: 3123123
│   ├─ received: 134123423
│   └─ channels: 
│       ├─ private: 245
│       ├─ groups: 89
│       └─ broadcasts: 12
├─ website: 
│   ├─ views: 12344
│   ├─ unique_users: 6345
│   └─ pages: 
│       ├─ home: 4521
│       ├─ about: 2341
│       └─ contact: 892
└─ email: 
    ├─ sent: 45234
    ├─ bounced: 234
    └─ campaigns: [3 items]
        ├─ newsletter: {...}
        └─ promotions: {...}
```

## Data Set Configuration

When a file with multiple data sets is opened, you can configure each one individually.

```
🔧 Configuration Setup - Data Set 2/3
============================================================
Configuring: products

Progress: ✓ ● ○ (2/3) [Configured: 1, Skipped: 0]

Choose representation:
[t] Table view   [b] Bar chart   [s] Skip this data set

Navigation:
[←] Previous   [→] Next   [q] Quit
```

- **Progress Bar**: Shows completed (✓), current (●), pending (○), and skipped (➖) data sets.
- **Navigation**: Use arrow keys `←` and `→` to move between data sets. Navigating away without configuring will mark the current set as skipped.

## Slides/Pages Configuration

Organize your data sets into separate slides for better presentation and navigation:

```
🔧 Configuration Setup - Data Set 2/4
============================================================
Configuring: sales_data

Choose representation:
[t] Table view - Shows data in tabular format
[b] Bar chart - Shows data as horizontal bars
[r] Tree view - Shows nested data hierarchically
[s] Skip - Omit this data set from display

Slide options:
[1-9] Put on slide number (1-9)
[n] Create new slide for this data set

Navigation:
[←] Previous data set
[→] Next data set
[q] Quit configuration
```

### Slide Organization Summary
After configuration, see how your data sets are organized:

```
🎉 Configuration Complete!
==================================================
Total data sets: 4
Configured for display: 4
Skipped: 0

📊 Data sets organized by slides:

📄 Slide 1:
  • users
    View: Table
    Columns: name, age, department
  • products
    View: Table
    Columns: product_name, price, category

📄 Slide 2:
  • sales_data
    View: Bars
    Field: revenue
  • departments
    View: Bars
    Field: employees
```

### Slide Navigation During Viewing
Navigate between slides using arrow keys:

```
============================================================
VSR - data.json | Slide 1/2 (2 data sets)
============================================================

[Data visualization content for slide 1]

------------------------------------------------------------
Showing lines 1-15 of 28
[j/k]Scroll [g/G]Top/Bottom [←/→]Slides [r]Refresh [c]Config [h]Help [q]Quit
```

## Key Bindings

| Context                  | Key(s)              | Action                                       |
| ------------------------ | ------------------- | -------------------------------------------- |
| **Main View**            | `j` / `k`           | Scroll down / up                             |
|                          | `g` / `G`           | Go to top / bottom                           |
|                          | `←` / `→`           | Navigate to previous / next slide            |
|                          | `h`                 | Show help screen                             |
|                          | `r` / `Ctrl+L`      | Refresh / redraw screen                      |
|                          | `c`                 | Reconfigure data sets                        |
|                          | `q`                 | Quit application                             |
| **File Selection**       | `↑` / `↓`           | Navigate files                               |
|                          | `Enter`             | Select highlighted file                      |
|                          | `c`                 | Clean up configs for missing files           |
|                          | `r`                 | Refresh file list                            |
| **Data Set Config**      | `t` / `b` / `r` / `s` | Set view to Table, Bar, Tree, or Skip        |
|                          | `1-9`               | Assign to slide number (1-9)                |
|                          | `n`                 | Create new slide                             |
|                          | `←` / `→`           | Go to previous / next data set               |
| **Column Selection**     | `↑` / `↓`           | Navigate columns                             |
|                          | `Space`             | Toggle selection                             |
|                          | `a` / `n`           | Select all / none                            |
|                          | `Enter`             | Confirm (requires ≥2 selected)               |
| **Bar Field Selection**  | `↑` / `↓`           | Navigate fields                              |
|                          | `Space`             | Select field (only one)                      |
|                          | `Enter`             | Confirm selection                            |

## Testing

The application includes a comprehensive test suite covering core functionality, configuration management, and all interactive interfaces.

## License

This project is open source and available under the MIT License.
