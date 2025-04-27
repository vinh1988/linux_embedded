# High-Level System Diagrams

This document explains the high-level diagrams that provide different views of the Sensor Gateway System architecture.

## Available Diagrams

### 1. Component Diagram (`component_diagram.png`)

This diagram shows the main components of the system and their interactions at a high level. It focuses on:
- The main system components
- Data flow between components
- Shared resources
- External interfaces

This diagram is useful for getting a quick overview of the system architecture without getting lost in implementation details.

### 2. Module Diagram (`module_diagram.png`)

This diagram shows the relationships between source files and header files in the codebase. It illustrates:
- Source file dependencies
- Header file usage
- External library dependencies

This diagram is useful for understanding the code organization and module dependencies.

### 3. Layered Architecture Diagram (`layered_diagram.png`)

This diagram presents the system as a set of layers, showing:
- External systems layer (sensor nodes, visualization tools, user interface)
- Interface layer (TCP sockets, command line, FIFO)
- Core components layer (connection manager, data manager, etc.)
- Data layer (shared memory, database, log file, etc.)

This diagram is useful for understanding the system from an architectural perspective, showing how different layers interact.

### 4. Thread Communication Diagram (`thread_diagram.png`)

This diagram focuses on the threads and processes in the system and how they communicate:
- Thread creation relationships
- Inter-thread communication
- Shared data access patterns
- Process boundaries

This diagram is useful for understanding the concurrency model of the system.

## Using These Diagrams

These high-level diagrams complement the more detailed function maps by providing different perspectives on the system:

1. **Start with the Component Diagram** to get a high-level overview of the system
2. **Use the Layered Architecture Diagram** to understand the system's architectural organization
3. **Refer to the Thread Communication Diagram** to understand concurrency and data flow
4. **Use the Module Diagram** to understand code organization
5. **Dive into the detailed function maps** when you need to understand specific implementation details

## Generating the Diagrams

To regenerate these diagrams:

```bash
./generate_high_level_diagrams.py
```

This will create:
- `component_diagram.png`: High-level component view
- `module_diagram.png`: Module dependency view
- `layered_diagram.png`: Layered architecture view
- `thread_diagram.png`: Thread communication view

If PNG generation fails for any diagram, an HTML file will be created instead, which can be opened in a web browser to view the diagram.
