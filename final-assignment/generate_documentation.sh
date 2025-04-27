#!/bin/bash

echo "Generating documentation for Sensor Gateway System..."

# Make scripts executable
chmod +x generate_workflow_diagram.py
chmod +x generate_temperature_image.py
chmod +x generate_mermaid_diagram.py
chmod +x generate_function_maps.py
chmod +x generate_high_level_diagrams.py

# Generate workflow diagram
echo "Generating workflow diagram..."
if command -v python3 &> /dev/null; then
    if python3 -c "import matplotlib" &> /dev/null; then
        ./generate_workflow_diagram.py
    else
        echo "Warning: matplotlib not installed. Skipping workflow diagram generation."
        echo "To install: pip3 install matplotlib"
    fi
else
    echo "Warning: Python 3 not found. Skipping workflow diagram generation."
fi

# Generate temperature image
echo "Generating temperature measurement image..."
if command -v python3 &> /dev/null; then
    if python3 -c "import matplotlib" &> /dev/null; then
        ./generate_temperature_image.py --output temperature_measurement.png
    else
        echo "Warning: matplotlib not installed. Skipping temperature image generation."
        echo "To install: pip3 install matplotlib"
    fi
else
    echo "Warning: Python 3 not found. Skipping temperature image generation."
fi

# Generate Mermaid diagram
echo "Generating Mermaid workflow diagram..."
if command -v python3 &> /dev/null; then
    if python3 -c "import requests" &> /dev/null; then
        ./generate_mermaid_diagram.py
    else
        echo "Warning: requests module not installed. Skipping Mermaid diagram generation."
        echo "To install: pip3 install requests"
    fi
else
    echo "Warning: Python 3 not found. Skipping Mermaid diagram generation."
fi

# Generate Function Maps
echo "Generating function relationship maps..."
if command -v python3 &> /dev/null; then
    if python3 -c "import requests" &> /dev/null; then
        ./generate_function_maps.py
    else
        echo "Warning: requests module not installed. Skipping function map generation."
        echo "To install: pip3 install requests"
    fi
else
    echo "Warning: Python 3 not found. Skipping function map generation."
fi

# Generate High-Level Diagrams
echo "Generating high-level system diagrams..."
if command -v python3 &> /dev/null; then
    if python3 -c "import requests" &> /dev/null; then
        ./generate_high_level_diagrams.py
    else
        echo "Warning: requests module not installed. Skipping high-level diagram generation."
        echo "To install: pip3 install requests"
    fi
else
    echo "Warning: Python 3 not found. Skipping high-level diagram generation."
fi

# List generated documentation
echo -e "\nGenerated documentation:"
echo "- WORKFLOW_README.md: Detailed explanation of system workflow"
echo "- workflow_diagram.png: Visual representation of system workflow (if matplotlib installed)"
echo "- workflow_mermaid.png: Mermaid-based workflow diagram (if requests installed)"
echo "- workflow_mermaid.html: HTML version of Mermaid diagram (if PNG generation failed)"
echo "- FUNCTION_MAP_README.md: Explanation of function relationship maps"
echo "- function_map.png: High-level function relationship map (if requests installed)"
echo "- detailed_function_map.png: Detailed function relationship map (if requests installed)"
echo "- simple_function_map.png: Simplified function relationship map (if requests installed)"
echo "- HIGH_LEVEL_DIAGRAMS_README.md: Explanation of high-level system diagrams"
echo "- component_diagram.png: Component-level system diagram (if requests installed)"
echo "- module_diagram.png: Module dependency diagram (if requests installed)"
echo "- layered_diagram.png: Layered architecture diagram (if requests installed)"
echo "- thread_diagram.png: Thread communication diagram (if requests installed)"
echo "- ANIMATION_README.md: Instructions for using the visualization tools"
echo "- temperature_measurement.png: Example temperature visualization (if matplotlib installed)"

echo -e "\nDone!"
