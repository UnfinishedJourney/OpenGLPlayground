OpenGLApplication
Overview
OpenGLApplication is my personal project focused on implementing core features of modern OpenGL. This project serves as a platform to explore and demonstrate essential graphics programming techniques.

Features
Rendering: Graphics rendering using OpenGL and GLSL.
Programming Language: Written in C++.
User Interface: Integrated with ImGui for interactive GUI controls.
Model Loading: Supports various 3D models with Assimp.
Texture Handling: Utilizes stb_image for loading textures.
Mathematics: Employs GLM for mathematical operations.
Window Management: Managed by GLFW for window creation and input handling.
OpenGL Function Loading: Handled by GLAD for accessing OpenGL functions.
Assets have been taken from https://github.com/KhronosGroup/glTF-Sample-Assets.git.
For JSON parsing (https://github.com/nlohmann/json/releases)

Project structure should look something like this
src/
├── Application/
│   ├── Application.h/.cpp
│   ├── InputManager.h/.cpp
│   ├── OpenGLContext.h/.cpp
│   └── **Window.h/.cpp**
├── Renderer/
│   ├── Renderer.h/.cpp
│   ├── **RenderPass.h/.cpp**
│   ├── **RenderState.h/.cpp**
│   └── **RenderObject.h/.cpp**
├── Graphics/
│   ├── Buffers/
│   │   ├── VertexBuffer.h/.cpp
│   │   ├── IndexBuffer.h/.cpp
│   │   ├── VertexArray.h/.cpp
│   │   ├── **UniformBuffer.h/.cpp**
│   │   └── **FrameBuffer.h/.cpp**
│   ├── Shaders/
│   │   ├── Shader.h/.cpp
│   │   ├── ShaderProgram.h/.cpp
│   │   ├── ShaderManager.h/.cpp
│   │   └── **ComputeShader.h/.cpp**
│   ├── Textures/
│   │   ├── Texture.h/.cpp
│   │   ├── Texture2D.h/.cpp
│   │   ├── TextureCube.h/.cpp
│   │   └── **TextureManager.h/.cpp**
│   ├── Materials/
│   │   ├── Material.h/.cpp
│   │   └── **MaterialManager.h/.cpp**
│   ├── Meshes/
│   │   ├── Mesh.h/.cpp
│   │   ├── Model.h/.cpp
│   │   ├── **MeshLoader.h/.cpp**
│   │   └── **PrimitiveShapes.h/.cpp**
│   └── States/
│       ├── **RenderState.h/.cpp**
│       └── **SamplerState.h/.cpp**
├── Scene/
│   ├── Scene.h/.cpp
│   ├── Camera.h/.cpp
│   ├── Light.h/.cpp
│   ├── Transform.h/.cpp
│   ├── **Node.h/.cpp**
│   └── **SceneGraph.h/.cpp**
├── Resources/
│   ├── ResourceManager.h/.cpp
│   ├── **FileSystem.h/.cpp**
│   └── **AssetLoader.h/.cpp**
├── Utilities/
│   ├── Utility.h/.cpp
│   └── **Logger.h/.cpp**
├── Tests/
│   ├── AllTests.h
│   ├── Test.h/.cpp
│   ├── TestAssimp.h/.cpp
│   ├── TestClearColor.h/.cpp
│   ├── TestComputeShader.h/.cpp
│   ├── TestInstance.h/.cpp
│   ├── TestLights.h/.cpp
│   └── **(другие тесты)**
└── stb_image/
    ├── stb_image.h
    └── stb_image.cpp