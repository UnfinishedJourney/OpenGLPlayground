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



opengl extentions

a. GL_EXT_texture_filter_anisotropic
Purpose: Enhances texture sampling quality at oblique viewing angles through anisotropic filtering.
Benefits: Reduces blur and preserves detail in textures viewed at sharp angles.
b. GL_ARB_direct_state_access (DSA)
Purpose: Simplifies OpenGL function calls by allowing direct modification of objects without binding them first.
Benefits: Cleaner and more efficient code, reducing state changes and potential errors.
c. GL_ARB_shader_storage_buffer_object (SSBO)
Purpose: Provides high-capacity buffer storage accessible in shaders.
Benefits: Facilitates complex data structures and large datasets in shaders, essential for PBR and advanced lighting models.
d. GL_ARB_bindless_texture
Purpose: Allows shaders to access textures without binding them to specific texture units.
Benefits: Reduces the overhead of texture binding, enabling more efficient texture management, especially with a large number of textures.
e. GL_ARB_multi_draw_indirect
Purpose: Enables issuing multiple draw commands with a single function call.
Benefits: Reduces CPU-GPU synchronization and draw call overhead, enhancing rendering performance.
f. GL_ARB_texture_compression
Purpose: Supports compressed texture formats (e.g., S3TC/DXT, ASTC).
Benefits: Reduces GPU memory usage and bandwidth, improving performance without compromising visual quality.
g. GL_ARB_sync
Purpose: Provides synchronization primitives to coordinate operations between the CPU and GPU.
Benefits: Enhances rendering pipeline efficiency by managing dependencies and avoiding unnecessary stalls.
3. Additional Beneficial Extensions
Beyond the extensions commonly recommended, here are a few more that can significantly enhance your rendering pipeline:

a. GL_ARB_compute_shader
Purpose: Introduces compute shaders, enabling general-purpose computing on the GPU.
Benefits: Facilitates parallel processing tasks like physics simulations, particle systems, and image processing.
b. GL_EXT_framebuffer_sRGB
Purpose: Enables framebuffer sRGB encoding for accurate color representation.
Benefits: Ensures that colors are correctly interpreted and displayed, crucial for PBR workflows.
c. GL_ARB_timer_query
Purpose: Allows precise timing of GPU operations.
Benefits: Essential for profiling and optimizing rendering performance.
d. GL_ARB_separate_shader_objects
Purpose: Enables separate shader program objects for different shader stages.
Benefits: Offers more flexibility in shader management and reusability.
e. GL_ARB_multi_bind
Purpose: Allows binding multiple buffers or textures in a single call.
Benefits: Streamlines resource binding, reducing overhead.


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