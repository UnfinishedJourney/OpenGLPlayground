# OpenGLPlayground

![bistro1](https://github.com/UnfinishedJourney/OpenGLPlayground/blob/04f55b971486ef65b84f056367e6c8a23f25818f/bistro_screenshot1.png)

## Overview

OpenGLPlayground is a personal project dedicated to exploring modern OpenGL's core features. It serves as a learning platform, demonstrating various graphics programming techniques in real-time rendering while providing a foundation for further experimentation.

---

## Test Scenes

### 🏙️ Amazon Lumberyard Bistro

The Bistro Scene is used as a testing scene for rendering techniques and materials. Below are some screenshots showcasing the environment:

<div align="center">
  <img src="https://github.com/UnfinishedJourney/OpenGLPlayground/blob/a03f3ecdcb2d679cc4489410eff7b8c0396142df/bistro_screenshot2.png" width="400"/>
  <img src="https://github.com/UnfinishedJourney/OpenGLPlayground/blob/a03f3ecdcb2d679cc4489410eff7b8c0396142df/bistro_screenshot3.png" width="400"/>
</div>

#### Scene Resources
- **[Bistro Scene](https://casual-effects.com/data/)** – Model
- **[Bistro Materials](https://github.com/corporateshark/bistro_materials)** – Materials
- **[Environment Map](https://polyhaven.com/a/pizzo_pernice_puresky)** – HDRI equirectangular map

### 🛡️ Damaged Helmet

The Helmet Scene is used for evaluating PBR rendering, reflections, and material fidelity. Below are some screenshots showcasing the model:

<div align="center">
  <img src="https://github.com/UnfinishedJourney/OpenGLPlayground/blob/3af605a3553559edfdb8edb8d28e16daf305f06b/helmet_screenshot1.png" width="400"/>
  <img src="https://github.com/UnfinishedJourney/OpenGLPlayground/blob/3af605a3553559edfdb8edb8d28e16daf305f06b/helmet_screenshot2.png" width="400"/>
</div>

#### Scene Resources
- **[Helmet Model](https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/DamagedHelmet)** – Model
- **[Environment Map](https://polyhaven.com/a/lago_disola)** – HDRI equirectangular map

---


## Features

### 🎨 Rendering Techniques
- Physically Based Rendering (PBR) with irradiance mapping  
- Parallax and normal mapping  
- Shadow mapping for directional and spot lights  
- Cubemaps  
- Multiple lighting models (directional, spot lights)  
- Custom framebuffers and post-processing effects  

### ⚡ Performance & Optimization
- Efficient resource management for assets  
- Multisample anti-aliasing (MSAA)  
- Indirect rendering, Level-of-Detail (LOD), and batching  
- Scene Graph (disabled for static scenes)

### 🛠 Debugging & Profiling
- Integration with NVIDIA Nsight and RenderDoc  
- Performance analysis using [Easy Profiler](https://github.com/yse/easy_profiler)  
- Clang-tidy  

---

## 📦 Dependencies & External Resources

### 🔧 Core Dependencies  
| Feature | Library | Version |
|---------|---------|---------|
| Rendering | OpenGL 4.6 (GLAD, GL_ARB_bindless_texture), GLSL | - |
| Programming Language | ISO C++20 Standard | - |
| User Interface | [ImGui](https://github.com/ocornut/imgui) | 1.91.5 |
| Model Loading | [Assimp](https://github.com/assimp/assimp) | 5.4.3 |
| Texture Handling | [stb_image](https://github.com/nothings/stb) | rev 5c20573 |
| Mathematics | [GLM](https://github.com/g-truc/glm) | 1.0.1 |
| Window Management | [GLFW](https://github.com/glfw/glfw) | 3.3.4 |
| Function Loading | [GLAD](https://glad.dav1d.de/) | - |
| JSON Parsing | [nlohmann/json](https://github.com/nlohmann/json) | 3.11.2 |
| Logging | [spdlog](https://github.com/gabime/spdlog) | 1.15.0 |
| Mesh Optimization | [meshoptimizer](https://github.com/zeux/meshoptimizer) | 0.17 |
| Profiling | [easy_profiler](https://github.com/yse/easy_profiler) | - |

### 📂 Additional Resources  
- **Bootstrapping:** [Bootstrapping](https://github.com/corporateshark/bootstrapping) for dependency management  
- **Assets:**  
  - [glTF Sample Assets](https://github.com/KhronosGroup/glTF-Sample-Assets)  
  - [Flipbooks](https://unity.com/blog/engine-platform/free-vfx-image-sequences-flipbooks)  
- **Materials & Textures:**  
  - [Polyhaven](https://polyhaven.com/) for high-quality cubemaps  

---

## 💻 Development Environment

### 📌 Prerequisites  
Ensure that you have the following installed:  
- A C++ compiler with C++20 support (tested on Clang 19.1.1, MSVC 19.43.34808)  
- CMake (tested on 3.29.2)  
- Python (tested on 3.12.0)  
- A GPU that supports OpenGL 4.6 and ARB_bindless_texture  
- OS: Windows 11  
- Visual Studio 2022  

---

## 🚀 Getting Started

### ⚙️ Setup Instructions

1. **Clone the Repository**  
   ```bash
   git clone https://github.com/UnfinishedJourney/OpenGLPlayground.git
   cd OpenGLPlayground
   ```

2. **Bootstrap Dependencies**  
   Clone `bootstrap.py` from its repository at [Bootstrapping](https://github.com/corporateshark/bootstrapping) to the `deps` folder.  
   Run:  
   ```bash
   python bootstrap.py
   ```

3. **STB Setup**  
   Create a `deps/src/stb_image` folder and move `stb_image.h`, `stb_image_resize2.h`, and `stb_image_write.h` files there.  

4. **Install Easy Profiler**  
   Download and install [Easy Profiler](https://github.com/yse/easy_profiler) (along with Qt6).  
   Configure CMake settings:  
   ```cmake
   set(EASY_PROFILER_INCLUDE_DIR "...")
   set(EASY_PROFILER_LIB_DIR "...")
   set(EASY_PROFILER_DLL "...")
   ```

5. **Install GLAD**  
   Download [GLAD](https://glad.dav1d.de/) with settings: C/C++, OpenGL 4.6, GL_ARB_bindless_texture, Core profile.  
   Place it in `deps/src/GLAD`.  

6. **Download Assets**  
   - Create an `assets` folder.  
   - Download a cubemap of your choice and update `textures_config.json` with its name under `currentSkybox`.  
   - Download models and update paths in `models_config.json`.  

7. **Configure the Project with CMake**  
   Example:
   ```bash
   mkdir build
   cd build
   cmake .. -G "Visual Studio 17 2022" -A x64
   cmake --build . --config Release
   ```

8. **Run the Application**  
   Open the `.sln` file in Visual Studio and run the project.  

---

## 📚 Books & Learning Resources

| Title | Author(s) | Links |
|-------|----------|-------------|
| The Cherno's OpenGL Tutorial Series | Yan Chernikov | [YouTube Playlist](https://www.youtube.com/playlist?list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2) |
| LearnOpenGL | Joey de Vries | [LearnOpenGL](https://learnopengl.com/) |
| OpenGL 4 Shading Language Cookbook | David Wolff | [O'Reilly](https://learning.oreilly.com/library/view/opengl-4-shading/9781789342253/) |
| 3D Graphics Rendering Cookbook | Sergey Kosarevsky, Viktor Latypov | [O'Reilly](https://learning.oreilly.com/library/view/3d-graphics-rendering/9781838986193/) |
| Game Engine Architecture (3rd Edition) | Jason Gregory | [O'Reilly](https://learning.oreilly.com/library/view/game-engine-architecture/9781351974271/) |
| GPU Pro 6: Advanced Rendering Techniques | Wolfgang Engel (Editor) | [O'Reilly](https://learning.oreilly.com/library/view/gpu-pro-6/9781482264623/) |

