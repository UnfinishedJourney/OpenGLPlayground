# OpenGLApplication

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)](https://github.com/yourusername/OpenGLApplication)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

## Overview

**OpenGLApplication** is a personal project dedicated to exploring the core features of modern OpenGL. Serving as a learning platform, it demonstrates a range of graphics programming techniques in real-time rendering while providing a foundation for further experimentation. Although the project implements many advanced techniques, its focus is on personal growth and understanding rather than being a full-scale production engine.

## Features

- **Cubemap and Texture Atlases:** Efficient handling of environment maps and texture atlases.
- **Shader Types:** Implementation of vertex, fragment, geometry, and compute shaders.
- **PBR & Irradiance Map:** Support for Physically Based Rendering (PBR) and irradiance mapping for realistic lighting.
- **Mapping Techniques:** Parallax mapping and normal mapping to enhance surface detail.
- **Shadow Maps:** Dynamic shadow mapping for directional and spot lights.
- **Lighting:** Multiple lighting models including directional and spot lights.
- **Resource Management:** Various resource managers to efficiently handle assets.
- **MSAA:** Multisample anti-aliasing for smoother visuals.
- **Framebuffers & Postprocessing:** Custom framebuffers and a range of postprocessing effects.

## Dependencies and Additional Resources

The project relies on several third-party libraries and external resources. Dependencies are managed via a bootstrap process, and key components include:

- **Rendering:** OpenGL 4.6 (GLAD) and GLSL  
- **Programming Language:** C++20  
- **User Interface:** [ImGui](https://github.com/ocornut/imgui) (v1.91.5)  
- **Model Loading:** [Assimp](https://github.com/assimp/assimp) (v5.4.3)  
- **Texture Handling:** stb_image ([stb](https://github.com/nothings/stb), revision 5c20573)  
- **Mathematics:** [GLM](https://github.com/g-truc/glm) (v1.0.1)  
- **Window Management:** [GLFW](https://github.com/glfw/glfw) (v3.3.4)  
- **OpenGL Function Loading:** [GLAD](https://glad.dav1d.de/) (manually downloaded)  
- **JSON Parsing:** [nlohmann/json](https://github.com/nlohmann/json) (v3.11.2)  
- **Logging:** [spdlog](https://github.com/gabime/spdlog) (v1.15.0)  
- **Mesh Optimization:** [meshoptimizer](https://github.com/zeux/meshoptimizer) (v0.17)  
- **Profiling:** Easy Profiler (installed manually with Qt6)  
- **Assets:** Sourced from the [glTF Sample Assets](https://github.com/KhronosGroup/glTF-Sample-Assets) repository and [Flipbooks](https://unity.com/blog/engine-platform/free-vfx-image-sequences-flipbooks)

Additional resources:
- **Bootstrapping:** This project uses [Bootstrapping](https://github.com/corporateshark/bootstrapping) for dependency management.
- **Bistro Materials:** Some assets and material definitions are based on resources from [Bistro Materials](https://github.com/corporateshark/bistro_materials) and data available at [Casual Effects](https://casual-effects.com/data/).
- **Cubemaps:** High-quality cubemaps can be found on [Polyhaven](https://polyhaven.com/).

## Development Environment

This project has been developed and tested using:
- **C++ Compilers:**  
  - Clang (with clang-tidy)  
    *Example:* `clang version 17.0.3` (Target: x86_64-pc-windows-msvc, Thread model: posix)
  - MSVC (Visual Studio Community 2022)  
    *Example:* Microsoft (R) C/C++ Optimizing Compiler Version 19.41.34123 for x64
- **Python:** 3.12.0
- **CMake:** 3.29.2
- **GPU:** NVIDIA RTX 3090

Ensure that you have:
- A C++ compiler with C++20 support (e.g., Clang, MSVC, or GCC)
- CMake (version 3.16 or higher)
- Python (version 3.6 or higher)
- Git for cloning repositories and managing dependencies

## Getting Started

### Setup Instructions

1. **Clone the Repository:**

   ```bash
   git clone https://github.com/yourusername/OpenGLApplication.git
   cd OpenGLApplication
Bootstrap Dependencies:

The project uses a bootstrap process defined in bootstrap.json:

bash
Copy
python bootstrap.py
Note: Replace bootstrap.py with the actual script name if it differs.

Manual Installation of Dependencies:

GLAD:
Download and extract the GLAD loader for OpenGL 4.6 Core Profile from the GLAD website, and place the glad directory in the appropriate location (e.g., external/).
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
Upon running the application, a window will open showcasing various graphics features. Use the integrated ImGui interface to:

Toggle and adjust rendering options.
Load and manage 3D models.
Modify camera settings.
Monitor performance using Easy Profiler.
Books and Learning Resources
3D Graphics Rendering Cookbook
A comprehensive guide to exploring rendering algorithms in modern OpenGL and Vulkan.
Authors: Sergey Kosarevsky, Viktor Latypov

OpenGL 4 Shading Language Cookbook
Build high-quality, real-time 3D graphics with OpenGL 4.6, GLSL 4.6, and C++17 (3rd Edition).
Author: David Wolff

Game Engine Architecture, Third Edition
An in-depth look at the design and implementation of game engine systems.
Author: Jason Gregory

The Cherno's OpenGL Tutorial Series
A beginner-friendly YouTube playlist that provided an early introduction to OpenGL fundamentals and 2D rendering.
Watch here

GPU Pro 6: Advanced Rendering Techniques
A collection of cutting-edge rendering techniques and best practices for modern game development (1st Edition).
Editor: Wolfgang Engel

License
OpenGLApplication is released under the MIT License.

Credits
Thanks to all the libraries, asset sources, and book authors whose work has contributed to this project:
