OpenGLApplication
Overview
OpenGLApplication is my personal project focused on implementing core features of modern OpenGL. This project serves as a platform to explore and demonstrate essential graphics programming techniques.

Features
Rendering: Graphics rendering using OpenGL 4.6 and GLSL.
Programming Language: Written in C++ 20.
User Interface: Integrated with ImGui for interactive GUI controls.
Model Loading: Supports various 3D models with Assimp.
Texture Handling: Utilizes stb_image for loading textures.
Mathematics: Employs GLM for mathematical operations.
Window Management: Managed by GLFW for window creation and input handling.
OpenGL Function Loading: Handled by GLAD for accessing OpenGL functions.
Assets have been taken from https://github.com/KhronosGroup/glTF-Sample-Assets.git.
For JSON parsing (https://github.com/nlohmann/json/releases)

Project structure should look something like this
# Project Files

## Application
- InputManager.cpp
- InputManager.h
- OpenGLApplication.cpp
- OpenGLContext.cpp
- OpenGLContext.h

## Graphics
### Buffers
- FrameBuffer.cpp
- FrameBuffer.h
- IndexBuffer.cpp
- IndexBuffer.h
- InstanceBuffer.cpp
- InstanceBuffer.h
- MeshBuffer.cpp
- MeshBuffer.h
- ShaderStorageBuffer.cpp
- ShaderStorageBuffer.h
- UniformBuffer.cpp
- UniformBuffer.h
- VertexArray.cpp
- VertexArray.h
- VertexBuffer.cpp
- VertexBuffer.h
- VertexBufferLayout.cpp
- VertexBufferLayout.h

### Materials
- Material.cpp
- Material.h

### Meshes
- AllMeshes.h
- Cube.cpp
- Cube.h
- Mesh.cpp
- Mesh.h
- MeshLayout.h
- Model.cpp
- Model.h
- Quad.cpp
- Quad.h
- SkyBox.cpp
- SkyBox.h
- Sphere.cpp
- Sphere.h

### Shaders
- BaseShader.cpp
- BaseShader.h
- ComputeShader.cpp
- ComputeShader.h
- Shader.cpp
- Shader.h
- UniformValue.h

### Textures
- CubeMapTexture.cpp
- CubeMapTexture.h
- Texture2D.cpp
- Texture2D.h
- TextureBase.cpp
- TextureBase.h

## Renderer
- Batch.cpp
- Batch.h
- BatchManager.cpp
- BatchManager.h
- RenderObject.cpp
- RenderObject.h
- RenderState.cpp
- RenderState.h
- Renderer.cpp
- Renderer.h

### Passes
- DebugLightsPass.cpp
- DebugLightsPass.h
- GeometryPass.cpp
- GeometryPass.h
- PostProcessingPass.cpp
- PostProcessingPass.h
- RenderPass.cpp
- RenderPass.h

## Resources
- MaterialManager.cpp
- MaterialManager.h
- ResourceManager.cpp
- ResourceManager.h
- ShaderManager.cpp
- ShaderManager.h

## Scene
- Camera.cpp
- Camera.h
- CameraController.cpp
- CameraController.h
- Lights.cpp
- Lights.h
- Scene.cpp
- Scene.h
- Screen.cpp
- Screen.h
- Transform.cpp
- Transform.h

## Utilities
- Logger.cpp
- Logger.h
- Utility.cpp
- Utility.h