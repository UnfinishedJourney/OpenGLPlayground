# OpenGLApplication

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)](https://github.com/yourusername/OpenGLApplication)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

## Overview

**OpenGLApplication** is a personal project dedicated to implementing and exploring the core features of modern OpenGL. This application serves as a platform to demonstrate essential graphics programming techniques and provides a foundation for learning and experimentation in real-time rendering and graphics development.

## Features

- **Rendering:** Advanced graphics rendering using OpenGL 4.6 and GLSL.
- **Programming Language:** Developed in C++20 for modern language features and performance.
- **User Interface:** Integrated with ImGui for interactive GUI controls.
- **Model Loading:** Supports various 3D model formats using Assimp.
- **Texture Handling:** Utilizes stb_image for efficient texture loading.
- **Mathematics:** Employs GLM for robust mathematical operations.
- **Window Management:** Managed by GLFW for window creation and input handling.
- **OpenGL Function Loading:** Handled by GLAD for accessing OpenGL functions.
- **JSON Parsing:** Utilizes nlohmann/json for easy JSON handling.
- **Logging:** Integrated with spdlog for efficient logging.
- **Mesh Optimization:** Uses meshoptimizer for optimized mesh processing.
- **Profiling:** Integrated with Easy Profiler for performance analysis (installed manually with Qt6).
- **Assets:** Assets sourced from the glTF Sample Assets repository.

## OpenGL Extensions

- **GL_ARB_bindless_texture:**  
  *Purpose:* Allows shaders to access textures without binding them to specific texture units.  
  *Benefits:* Reduces the overhead of texture binding, enabling more efficient texture management, especially with a large number of textures.

## Dependencies

The project utilizes several third-party libraries to provide essential functionalities. Dependencies are managed using the provided `bootstrap.json`, with some requiring manual installation.

### Managed via Bootstrap

- **meshoptimizer** (v0.17)
- **nlohmann/json** (v3.11.2)
- **Assimp** (v5.4.3)
- **spdlog** (v1.15.0)
- **GLFW** (v3.3.4)
- **GLM** (v1.0.1)
- **ImGui** (v1.91.5)
- **stb** (5c20573)

### Manually Installed

- **GLAD:** Downloaded manually from the [GLAD website](https://glad.dav1d.de/).
- **Easy Profiler with Qt6:** Installed manually alongside Qt6.

### Additional Requirements

- **Python:** Required for CMake scripts and other build-related tasks.
- **CMake:** Ensure you have CMake installed to configure and build the project.

## Getting Started

### Prerequisites

Ensure you have the following installed on your system:

- **C++ Compiler:** Supporting C++20 (e.g., GCC 10+, Clang 10+, MSVC 2019+).
- **CMake:** Version 3.16 or higher.
- **Python:** Version 3.6 or higher.
- **Git:** For cloning repositories and managing dependencies.

### Setup Instructions

1. **Clone the Repository:**

   ```bash
   git clone https://github.com/yourusername/OpenGLApplication.git
   cd OpenGLApplication
Bootstrap Dependencies:

The project uses a bootstrap process to manage dependencies defined in bootstrap.json.

bash
Copy
python bootstrap.py
Note: Replace bootstrap.py with the actual script name if it differs.

Manual Installation of Dependencies:

GLAD:
Download and extract the GLAD loader for OpenGL 4.6 Core Profile, then place the glad directory into the appropriate location (e.g., external/).
Easy Profiler with Qt6:
Follow the Easy Profiler installation guide for detailed instructions.
Configure the Project with CMake:

Create a build directory and run CMake:

bash
Copy
mkdir build
cd build
cmake ..
Optional: Specify the build type (Debug or Release):

bash
Copy
cmake -DCMAKE_BUILD_TYPE=Release ..
Build the Project:

bash
Copy
cmake --build .
Run the Application:

bash
Copy
./OpenGLApplication
Usage
Upon running the application, a window will appear showcasing various graphics rendering features. Use the integrated ImGui interface to:

Interact with different rendering options.
Load models.
Adjust camera settings.
Monitor performance through Easy Profiler.
References
Books
3D Graphics Rendering Cookbook
Author: [Author's Name]
Publisher: [Publisher Name], [Year]
Description: Provided practical recipes and techniques for rendering 3D graphics.

OpenGL 4 Shading Language Cookbook
Author: [Author's Name]
Publisher: [Publisher Name], [Year]
Description: Focused on the OpenGL Shading Language (GLSL) for advanced shader programming.

Game Engine Architecture, Third Edition
Author: [Author's Name]
Publisher: [Publisher Name], [Year]
Description: Offered insights into game engine design and architecture.

Online Tutorials
The Cherno's OpenGL Tutorial Series:
OpenGL Tutorial by The Cherno
Additional Resources
Good PBR Materials
Unreal Engine Presentation Notes
GPU Pro 6: Physically Based Light Probe Generation on GPU
License
OpenGLApplication is released under the MIT License.

Third-Party Licenses
meshoptimizer: MIT License
nlohmann/json: MIT License
Assimp: BSD License
spdlog: MIT License
GLFW: zlib/libpng License
GLM: The Happy Bunny License or MIT License
ImGui: MIT License
stb: Public Domain (or MIT License)
GLAD: MIT License
Easy Profiler: Refer to Easy Profiler's license
Acknowledgements
Special thanks to KhronosGroup glTF-Sample-Assets for providing sample assets.
Thanks to all contributors to the open-source libraries used in this project.