# OpenGLPlayground

## Overview

OpenGLPlayground is a personal project dedicated to exploring the core features of modern OpenGL. It serves as a learning platform, demonstrating various graphics programming techniques in real-time rendering while providing a foundation for further experimentation.

---

## Features

### 🎨 Rendering Techniques
- Shader types: vertex, fragment, geometry, and compute shaders  
- Physically Based Rendering (PBR) with irradiance mapping  
- Parallax and normal mapping
- Shadow mapping for directional and spot lights  
- Cubemaps
- Multiple lighting models (directional, spot)  
- Custom framebuffers and post-processing effects 

### 🚀 Performance & Optimization
- Resource management for efficient asset handling  
- Multisample anti-aliasing (MSAA)   
- Indirect rendering, Level-of-Detail (LOD), and batching  

### 🛠 Debugging & Profiling
- Integration with NVIDIA Nsight and RenderDoc  
- Performance analysis using [Easy Profiler](https://github.com/yse/easy_profiler) 
- Clang-tidy 

---

## 📦 Dependencies & External Resources

### 🔧 Core Dependencies  
| Feature                  | Library                                                   | Version |
|--------------------------|-----------------------------------------------------------|---------|
| Rendering               | OpenGL 4.6 (GLAD), GLSL, ARB_bindless_texture             | -       |
| Programming Language    | C++20                                                    | -       |
| User Interface         | [ImGui](https://github.com/ocornut/imgui)                 | 1.91.5  |
| Model Loading         | [Assimp](https://github.com/assimp/assimp)                 | 5.4.3   |
| Texture Handling      | [stb_image](https://github.com/nothings/stb)              | rev 5c20573 |
| Mathematics           | [GLM](https://github.com/g-truc/glm)                       | 1.0.1   |
| Window Management     | [GLFW](https://github.com/glfw/glfw)                       | 3.3.4   |
| Function Loading      | [GLAD](https://glad.dav1d.de/)    (installed manually with GL_ARB_BINDLESS extention)     | -       |
| JSON Parsing         | [nlohmann/json](https://github.com/nlohmann/json)          | 3.11.2  |
| Logging              | [spdlog](https://github.com/gabime/spdlog)                 | 1.15.0  |
| Mesh Optimization    | [meshoptimizer](https://github.com/zeux/meshoptimizer)     | 0.17    |
| Profiling            | [easy_profiler](https://github.com/yse/easy_profiler) (installed manually with Qt6) | - |

### 📂 Additional Resources  
- **Bootstrapping:** [Bootstrapping](https://github.com/corporateshark/bootstrapping) for dependency management  
- **Assets:**  
  - [glTF Sample Assets](https://github.com/KhronosGroup/glTF-Sample-Assets)  
  - [Flipbooks](https://unity.com/blog/engine-platform/free-vfx-image-sequences-flipbooks)  
- **Materials & Textures:**  
  - [Bistro Scene](https://casual-effects.com/data/)  
  - [Bistro Materials](https://github.com/corporateshark/bistro_materials)  
  - [Polyhaven](https://polyhaven.com/) for high-quality cubemaps  

---

## 💻 Development Environment

### 📌 Prerequisites  
Ensure that you have the following installed:  
- A C++ compiler with C++20 support (tested on Clang, MSVC)  
- CMake (tested on 3.29.2)  
- Python (tested on 3.12.0)  
- A GPU that supports OpenGL 4.6 and ARB_bindless_texture  
- OS: Windows 11  
- Vusual Studio (tested on 2022)
---

## 🏁 Getting Started

### ⚙️ Setup Instructions

1. **Clone the Repository**  
   ```bash
   git clone https://github.com/UnfinishedJourney/OpenGLPlayground.git
   cd OpenGLPlayground
   ```

2. **Bootstrap Dependencies**  
   Clone `bootstrap.py` from its Git repository at [Bootstrapping](https://github.com/corporateshark/bootstrapping).  
   to deps folder
   The project uses a bootstrap process defined in `bootstrap.json`. Run:  
   ```bash
   python bootstrap.py
   ```

3. **STB** 
   in  deps\src create stb_image folder 
   move stb_image.h, stb_image_resize2.h, stb_image_write.h files there
   
4. **Install Easy Profiler**  
   Download and install [Easy Profiler](https://github.com/yse/easy_profiler) (along with Qt6).  
   Configure CMake settings by specifying installation paths:  
   ```cmake
   set(EASY_PROFILER_INCLUDE_DIR "...")
   set(EASY_PROFILER_LIB_DIR "...")
   set(EASY_PROFILER_DLL "...")
   ```
5. **Install GLAD**  
   Download [GLAD](https://glad.dav1d.de/)
   C/C++, 4.6, GL_ARB_BINDLESS, OpeGL, Core
   Put it into the deps/src/GLAD folder

6. **Configure the Project with CMake**  
   Example :
   ```bash
   mkdir build
   cd build
   cmake .. -G "Visual Studio 17 2022" -A x64
   cmake --build . --config Release
   ```

7. **Run the Application**  
   Open the `.sln` file using Visual Studio and run the project.

---

## 📚 Books & Learning Resources

| Title | Author(s) | Description |
|-------|----------|-------------|
| The Cherno's OpenGL Tutorial Series | Yan Chernikov | [YouTube Playlist](https://www.youtube.com/playlist?list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2) |
| LearnOpenGL | Joey de Vries   | [LearnOpenGL](https://learnopengl.com/) |
| OpenGL 4 Shading Language Cookbook | David Wolff | [O'Reilly](https://learning.oreilly.com/library/view/opengl-4-shading/9781789342253/) |
| 3D Graphics Rendering Cookbook | Sergey Kosarevsky, Viktor Latypov | [O'Reilly](https://learning.oreilly.com/library/view/3d-graphics-rendering/9781838986193/) |
| Game Engine Architecture (3rd Edition) | Jason Gregory | [O'Reilly](https://learning.oreilly.com/library/view/game-engine-architecture/9781351974271/) |
| GPU Pro 6: Advanced Rendering Techniques | Wolfgang Engel (Editor) | [O'Reilly](https://learning.oreilly.com/library/view/gpu-pro-6/9781482264623/) |