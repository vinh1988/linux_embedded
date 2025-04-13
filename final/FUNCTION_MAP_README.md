# Function Relationship Maps

This document explains the function relationship maps for the Sensor Gateway System.

## Overview

The function relationship maps provide a visual representation of how the various functions in the system interact with each other. These maps help in understanding the code structure, dependencies, and flow of control within the system.

## Available Maps

### 1. High-Level Function Map (`function_map.md`)

This map provides a class-based overview of the system, showing the main components and their key functions. It's useful for getting a high-level understanding of the system architecture.

### 2. Detailed Function Map (`detailed_function_map.md`)

This map shows a detailed view of all the important functions in the system and their relationships. It includes:
- Function calls between components
- Data flow between functions
- Shared data structures
- Thread creation and synchronization

This map is useful for developers who need to understand the detailed interactions between system components.

### 3. Simplified Function Map (`simple_function_map.md`)

This map provides a simplified view of the system's function relationships, focusing on the key interactions while abstracting away some of the implementation details. It's useful for getting a clearer picture of the system's architecture without being overwhelmed by details.

## Generating the Maps

To generate the function maps:

```bash
./generate_function_maps.py
```

This will create:
- `function_map.png`: High-level function map
- `detailed_function_map.png`: Detailed function map
- `simple_function_map.png`: Simplified function map

If the PNG generation fails, HTML files will be created instead, which can be opened in a web browser to view the diagrams.

## Map Components

The function maps use color coding to distinguish between different system components:

- **Main Process** (pink): Functions in the main process
- **Connection Manager** (orange): Functions related to connection handling
- **Data Manager** (red): Functions for processing sensor data
- **Storage Manager** (dark red): Functions for database operations
- **Status Manager** (teal): Functions for system status monitoring
- **Security Manager** (light green): Functions for security features
- **Log Process** (light yellow): Functions for logging
- **Shared Components** (light yellow): Shared data structures and functions

## Understanding Function Relationships

The arrows in the diagrams represent function calls or data flow between components:

- A → B: Function A calls function B
- A → [Data Structure]: Function A accesses or modifies the data structure
- [Data Structure] → B: Function B reads from the data structure

## Using the Maps for Development

These maps are particularly useful for:

1. **Onboarding new developers**: Quickly understand the system architecture
2. **Debugging**: Trace the flow of execution through the system
3. **Code modifications**: Understand the impact of changes on other components
4. **Documentation**: Provide visual documentation of the system design

When making changes to the code, refer to these maps to understand the potential impact on other parts of the system.
