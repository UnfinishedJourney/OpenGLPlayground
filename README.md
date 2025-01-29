OpenGLApplication
Overview
OpenGLApplication is a personal project dedicated to implementing and exploring the core features of modern OpenGL. This application serves as a platform to demonstrate essential graphics programming techniques, providing a foundation for learning and experimentation in real-time rendering and graphics development.

Features
Rendering: Advanced graphics rendering using OpenGL 4.6 and GLSL.
Programming Language: Developed in C++20 for modern language features and performance.
User Interface: Integrated with ImGui for interactive GUI controls.
Model Loading: Supports various 3D model formats using Assimp.
Texture Handling: Utilizes stb_image for efficient texture loading.
Mathematics: Employs GLM for robust mathematical operations.
Window Management: Managed by GLFW for window creation and input handling.
OpenGL Function Loading: Handled by GLAD for accessing OpenGL functions.
JSON Parsing: Utilizes nlohmann/json for easy JSON handling.
Logging: Integrated with spdlog for efficient logging.
Mesh Optimization: Uses meshoptimizer for optimized mesh processing.
Profiling: Integrated with Easy Profiler for performance analysis (installed manually with Qt6).
Assets: Assets sourced from the glTF Sample Assets repository.


OpenGL Extensions
GL_ARB_bindless_texture
Purpose: Allows shaders to access textures without binding them to specific texture units.
Benefits: Reduces the overhead of texture binding, enabling more efficient texture management, especially with a large number of textures.

Dependencies
The project utilizes several third-party libraries to provide essential functionalities. Dependencies are managed using the provided bootstrap.json, with some requiring manual installation.

Managed via Bootstrap
The following dependencies are managed through the bootstrap process:

meshoptimizer (v0.17)
nlohmann/json (v3.11.2)
Assimp (v5.4.3)
spdlog (v1.15.0)
GLFW (v3.3.4)
GLM (v1.0.1)
ImGui (v1.91.5)
stb (5c20573)
Manually Installed
Some dependencies require manual installation:

GLAD: Downloaded manually from the GLAD website.
Easy Profiler with Qt6: Installed manually alongside Qt6.
Additional Requirements
Python: Required for CMake scripts and other build-related tasks.
CMake: Ensure you have CMake installed to configure and build the project.
Getting Started
Prerequisites
Ensure you have the following installed on your system:

C++ Compiler: Supporting C++20 (e.g., GCC 10+, Clang 10+, MSVC 2019+).
CMake: Version 3.16 or higher.
Python: Version 3.6 or higher.
Git: For cloning repositories and managing dependencies.
Setup Instructions
Clone the Repository:

bash
Copy
git clone https://github.com/yourusername/OpenGLApplication.git
cd OpenGLApplication
Bootstrap Dependencies:

The project uses a bootstrap process to manage dependencies defined in bootstrap.json. Ensure you have the necessary tools installed (e.g., Python and CMake).

bash
Copy
python bootstrap.py
Note: Replace bootstrap.py with the actual bootstrap script used in your project.

Manual Installation of Dependencies:

GLAD:

Download the latest GLAD loader for OpenGL 4.6 Core Profile from the GLAD website.
Extract and place the glad directory into the external/ folder or the appropriate location as per your project structure.
Easy Profiler with Qt6:

Download and install Easy Profiler compatible with Qt6.
Follow the Easy Profiler installation guide for detailed instructions.
Configure the Project with CMake:

Create a build directory and configure the project using CMake.

bash
Copy
mkdir build
cd build
cmake ..
Optional: Specify the build type (Debug or Release).

bash
Copy
cmake -DCMAKE_BUILD_TYPE=Release ..
Build the Project:

Compile the project using the generated build files.

bash
Copy
cmake --build .
Run the Application:

After a successful build, execute the application.

bash
Copy
./OpenGLApplication
Usage
Upon running the application, a window will appear showcasing various graphics rendering features. Use the integrated ImGui interface to interact with different rendering options, load models, adjust camera settings, and monitor performance through the Easy Profiler.

References
The following books and resources were instrumental in the development of this project:

Books
3D Graphics Rendering Cookbook
Author: [Author's Name]
Publisher: [Publisher Name], [Year]
Description: This cookbook provided practical recipes and techniques for rendering 3D graphics, offering insights into shader development, lighting models, and optimization strategies essential for the project's rendering pipeline.

OpenGL 4 Shading Language Cookbook
Author: [Author's Name]
Publisher: [Publisher Name], [Year]
Description: Focused on the OpenGL Shading Language (GLSL), this book was pivotal in understanding shader programming, enabling the implementation of advanced visual effects and graphics techniques within the application.

Game Engine Architecture, Third Edition
Author: [Author's Name]
Publisher: [Publisher Name], [Year]
Description: Provided a comprehensive overview of game engine design and architecture, influencing the structural decisions and component interactions within the OpenGLApplication project.

Online Tutorials
The Cherno's OpenGL Tutorial Series
Description: A comprehensive YouTube tutorial series that covers the fundamentals and advanced topics in OpenGL programming. Provided foundational knowledge and practical examples used throughout the project.
Link: OpenGL Tutorial by The Cherno

Good PBR materials:
https://google.github.io/filament/Filament.html#about/authors
https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
GPU Pro 6 Physically Based Light ProbeGeneration on GPU
License
OpenGLApplication is released under the MIT License.

Third-Party Licenses
This project includes several third-party libraries, each with their own licenses. Below is a summary of the licenses for the included dependencies:

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
For detailed information, please refer to each library's respective repository.

Acknowledgements
KhronosGroup glTF-Sample-Assets for providing sample assets.
All contributors to the open-source libraries used in this project.