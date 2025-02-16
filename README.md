# OpenGLApplication

## Overview

**OpenGLApplication** is a personal project dedicated to exploring the core features of modern OpenGL. It serves as a learning platform, demonstrating various graphics programming techniques in real-time rendering while providing a foundation for further experimentation.

---

## Features

### 🎨 Rendering Techniques
- Cubemap and texture atlases  
- Shader types: **vertex, fragment, geometry, and compute shaders**  
- **Physically Based Rendering (PBR)** with irradiance mapping  
- Parallax and normal mapping for enhanced surface details  
- Dynamic shadow mapping for directional and spot lights  

### 🚀 Performance & Optimization
- Multiple lighting models (**directional, spot**)  
- **Resource management** for efficient asset handling  
- **Multisample anti-aliasing (MSAA)**  
- Custom framebuffers and **post-processing effects**  
- **Indirect rendering, Level-of-Detail (LOD), and batching**  

### 🛠 Debugging & Profiling
- Integration with **NVIDIA Nsight** and **RenderDoc**  
- Performance analysis using [Easy Profiler](https://github.com/yse/easy_profiler)  

---

## 📦 Dependencies & External Resources

### 🔧 Core Dependencies  
| Feature                  | Library                                                   | Version |
|--------------------------|-----------------------------------------------------------|---------|
| **Rendering**            | OpenGL 4.6 (GLAD), GLSL                                  | -       |
| **Programming Language** | C++20                                                    | -       |
| **User Interface**       | [ImGui](https://github.com/ocornut/imgui)               | 1.91.5  |
| **Model Loading**        | [Assimp](https://github.com/assimp/assimp)               | 5.4.3   |
| **Texture Handling**     | [stb_image](https://github.com/nothings/stb)            | rev 5c20573 |
| **Mathematics**          | [GLM](https://github.com/g-truc/glm)                     | 1.0.1   |
| **Window Management**    | [GLFW](https://github.com/glfw/glfw)                     | 3.3.4   |
| **Function Loading**     | [GLAD](https://glad.dav1d.de/)                           | -       |
| **JSON Parsing**         | [nlohmann/json](https://github.com/nlohmann/json)        | 3.11.2  |
| **Logging**              | [spdlog](https://github.com/gabime/spdlog)               | 1.15.0  |
| **Mesh Optimization**    | [meshoptimizer](https://github.com/zeux/meshoptimizer)   | 0.17    |
| **Profiling**            | Easy Profiler (installed manually with Qt6)              | -       |

### 📂 Additional Resources  
- **Bootstrapping:** [Bootstrapping](https://github.com/corporateshark/bootstrapping) for dependency management  
- **Assets:**  
  - [glTF Sample Assets](https://github.com/KhronosGroup/glTF-Sample-Assets)  
  - [Flipbooks](https://unity.com/blog/engine-platform/free-vfx-image-sequences-flipbooks)  
- **Materials & Textures:**  
  - [Bistro Materials](https://github.com/corporateshark/bistro_materials)  
  - [Casual Effects](https://casual-effects.com/data/)  
  - [Polyhaven](https://polyhaven.com/) for high-quality cubemaps  

---

## 💻 Development Environment

### 🛠 Hardware & Software Requirements  
- **C++ Compilers:**  
  - Clang (with clang-tidy) – `clang version 17.0.3` (Target: x86_64-pc-windows-msvc, Thread model: posix)  
  - MSVC (Visual Studio Community 2022) – `C/C++ Optimizing Compiler Version 19.41.34123 for x64`  
- **Other Tools:**  
  - **Python:** 3.12.0  
  - **CMake:** 3.29.2  
  - **GPU:** NVIDIA RTX 3090  

### 📌 Prerequisites  
Ensure that you have the following installed:  
- A **C++ compiler with C++20** support (Clang, MSVC, or GCC)  
- **CMake** (v3.16 or higher)  
- **Python** (v3.6 or higher)  
- **Git** for cloning repositories and managing dependencies  

---

## 🚀 Getting Started

### ⚙️ Setup Instructions

1. **Clone the Repository**  
   ```bash
   git clone https://github.com/UnfinishedJourney/OpenGLApplication.git
   cd OpenGLApplication
   ```

2. **Bootstrap Dependencies**  
   Clone `bootstrap.py` from its Git repository at [Bootstrapping](https://github.com/corporateshark/bootstrapping).  
   The project uses a bootstrap process defined in `bootstrap.json`. Run:  
   ```bash
   python bootstrap.py
   ```
   *(Replace `bootstrap.py` with the actual script name if it differs.)*

3. **Install Easy Profiler**  
   Download and install [Easy Profiler](https://github.com/yse/easy_profiler) (along with Qt6).  
   Configure CMake settings by specifying installation paths:  
   ```cmake
   set(EASY_PROFILER_INCLUDE_DIR "C:/Program Files (x86)/easy_profiler/include")
   set(EASY_PROFILER_LIB_DIR "C:/Program Files (x86)/easy_profiler/lib")
   set(EASY_PROFILER_DLL "C:/Program Files (x86)/easy_profiler/bin/easy_profiler.dll")
   ```

4. **Configure the Project with CMake**  
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

5. **Run the Application**  
   ```bash
   ./OpenGLApplication
   ```

---

## 📚 Books & Learning Resources

| Title | Author(s) | Description |
|-------|----------|-------------|
| **The Cherno's OpenGL Tutorial Series** | YouTube Playlist | Beginner-friendly introduction to OpenGL fundamentals |
| **OpenGL 4 Shading Language Cookbook** | David Wolff | Advanced shading techniques with OpenGL 4.6 and GLSL 4.6 |
| **3D Graphics Rendering Cookbook** | Sergey Kosarevsky, Viktor Latypov | A guide to rendering algorithms in OpenGL and Vulkan |
| **Game Engine Architecture (3rd Edition)** | Jason Gregory | In-depth study of game engine systems |
| **GPU Pro 6: Advanced Rendering Techniques** | Wolfgang Engel (Editor) | Collection of rendering techniques for modern game development |

---

## 🎖 Credits

Special thanks to all the libraries, asset sources, and book authors whose work has contributed to this project.