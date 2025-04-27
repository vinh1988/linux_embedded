#!/usr/bin/env python3
import os
import subprocess
import sys
import requests
import base64
import json
import urllib.parse

def check_mmdc_installed():
    """Check if mmdc (mermaid-cli) is installed"""
    try:
        subprocess.run(['mmdc', '--version'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        return True
    except FileNotFoundError:
        return False

def generate_with_mmdc(input_file, output_file):
    """Generate PNG using mermaid-cli"""
    try:
        subprocess.run(['mmdc', '-i', input_file, '-o', output_file], check=True)
        print(f"Successfully generated {output_file} using mermaid-cli")
        return True
    except subprocess.CalledProcessError as e:
        print(f"Error generating diagram with mermaid-cli: {e}")
        return False

def generate_with_kroki(input_file, output_file):
    """Generate PNG using Kroki API"""
    try:
        # Read the mermaid code from the file
        with open(input_file, 'r') as f:
            content = f.read()
        
        # Extract the mermaid code from the markdown code block
        if '```mermaid' in content:
            mermaid_code = content.split('```mermaid')[1].split('```')[0].strip()
        else:
            mermaid_code = content
        
        # Prepare the request to Kroki API
        payload = {
            "diagram_source": mermaid_code,
            "diagram_type": "mermaid",
            "output_format": "png"
        }
        
        # Send the request
        response = requests.post("https://kroki.io/mermaid/png", json=payload)
        
        if response.status_code == 200:
            with open(output_file, 'wb') as f:
                f.write(response.content)
            print(f"Successfully generated {output_file} using Kroki API")
            return True
        else:
            print(f"Error: Kroki API returned status code {response.status_code}")
            return False
    except Exception as e:
        print(f"Error generating diagram with Kroki API: {e}")
        return False

def generate_with_api(input_file, output_file):
    """Generate PNG using mermaid.ink API"""
    try:
        # Read the mermaid code from the file
        with open(input_file, 'r') as f:
            content = f.read()
        
        # Extract the mermaid code from the markdown code block
        if '```mermaid' in content:
            mermaid_code = content.split('```mermaid')[1].split('```')[0].strip()
        else:
            mermaid_code = content
        
        # Encode the mermaid code for the URL
        encoded = base64.b64encode(mermaid_code.encode('utf-8')).decode('utf-8')
        
        # Create the URL for the mermaid.ink service
        url = f"https://mermaid.ink/img/{encoded}"
        
        # Download the image
        response = requests.get(url)
        if response.status_code == 200:
            with open(output_file, 'wb') as f:
                f.write(response.content)
            print(f"Successfully generated {output_file} using mermaid.ink API")
            return True
        else:
            print(f"Error: API returned status code {response.status_code}")
            return False
    except Exception as e:
        print(f"Error generating diagram with API: {e}")
        return False

def generate_html_with_mermaid_js(input_file, output_file):
    """Generate an HTML file with mermaid.js that can be opened in a browser"""
    try:
        # Read the mermaid code from the file
        with open(input_file, 'r') as f:
            content = f.read()
        
        # Extract the mermaid code from the markdown code block
        if '```mermaid' in content:
            mermaid_code = content.split('```mermaid')[1].split('```')[0].strip()
        else:
            mermaid_code = content
        
        # Get the base filename without extension
        base_name = os.path.splitext(os.path.basename(input_file))[0]
        
        # Create HTML with mermaid.js
        html_content = f"""
        <!DOCTYPE html>
        <html>
        <head>
            <meta charset="UTF-8">
            <title>{base_name} - Function Map</title>
            <script src="https://cdn.jsdelivr.net/npm/mermaid/dist/mermaid.min.js"></script>
            <script>
                mermaid.initialize({{
                    startOnLoad: true,
                    theme: 'default',
                    securityLevel: 'loose',
                }});
            </script>
            <style>
                body {{
                    font-family: Arial, sans-serif;
                    margin: 20px;
                    text-align: center;
                }}
                .mermaid {{
                    margin: 0 auto;
                    max-width: 100%;
                }}
                .instructions {{
                    margin-top: 20px;
                    padding: 10px;
                    background-color: #f8f9fa;
                    border-radius: 5px;
                    display: inline-block;
                }}
            </style>
        </head>
        <body>
            <h1>{base_name} - Function Map</h1>
            <div class="mermaid">
{mermaid_code}
            </div>
            <div class="instructions">
                <p>To save as PNG:</p>
                <ol>
                    <li>Right-click on the diagram</li>
                    <li>Select "Save image as..."</li>
                    <li>Save with filename "{os.path.basename(output_file)}"</li>
                </ol>
            </div>
        </body>
        </html>
        """
        
        # Save the HTML file
        html_file = output_file.replace('.png', '.html')
        with open(html_file, 'w') as f:
            f.write(html_content)
        
        print(f"Generated HTML file: {html_file}")
        print("Open this file in a web browser and save the diagram as an image.")
        return True
    except Exception as e:
        print(f"Error generating HTML: {e}")
        return False

def generate_function_map(input_file, output_file):
    """Generate a function map diagram from a Mermaid file"""
    print(f"Generating function map: {output_file}")
    
    if not os.path.exists(input_file):
        print(f"Error: Input file {input_file} not found")
        return False
    
    # Try different methods in order of preference
    if check_mmdc_installed():
        if generate_with_mmdc(input_file, output_file):
            return True
    
    print("mermaid-cli not found, trying API methods...")
    
    if generate_with_kroki(input_file, output_file):
        return True
    
    if generate_with_api(input_file, output_file):
        return True
    
    print("API methods failed, generating HTML file that can be opened in a browser...")
    generate_html_with_mermaid_js(input_file, output_file)
    
    print("\nTo install mermaid-cli for better results:")
    print("1. Install Node.js and npm")
    print("2. Run: npm install -g @mermaid-js/mermaid-cli")
    
    return False

def main():
    # Define the function maps to generate
    function_maps = [
        {"input": "function_map.md", "output": "function_map.png"},
        {"input": "detailed_function_map.md", "output": "detailed_function_map.png"},
        {"input": "simple_function_map.md", "output": "simple_function_map.png"}
    ]
    
    # Generate each function map
    for map_info in function_maps:
        generate_function_map(map_info["input"], map_info["output"])

if __name__ == "__main__":
    main()
