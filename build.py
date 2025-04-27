import json
import os
import subprocess
import sys

def main():
    # Load configuration
    with open('build_config.json', 'r') as f:
        config = json.load(f)
    
    # Create build directory if it doesn't exist
    if not os.path.exists(config['project']['build_dir']):
        os.makedirs(config['project']['build_dir'])
    
    # Construct compiler command
    compiler = "g++" if sys.platform != "win32" else "g++"  # Use g++ for both Windows and Linux for simplicity
    
    # Prepare include paths
    includes = [f"-I{path}" for path in config['include_paths']]
    
    # Prepare source files
    sources = config['source_files'] + [config['dependencies']['sqlite']['source']]
    
    # Construct the full build command
    build_cmd = [
        compiler,
        *config['compiler_flags'],
        *includes,
        *sources,
        *config['linker_flags'],
        "-o", os.path.join(config['project']['build_dir'], config['project']['output'])
    ]
    
    # Execute the build command
    print("Building with command:")
    print(" ".join(build_cmd))
    
    try:
        subprocess.check_call(build_cmd)
        print(f"\nBuild successful! Executable created at: {os.path.join(config['project']['build_dir'], config['project']['output'])}")
    except subprocess.CalledProcessError as e:
        print(f"\nBuild failed with error code {e.returncode}")
        sys.exit(1)
    
if __name__ == "__main__":
    main()