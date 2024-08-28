cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(OpenGLApplication VERSION 1.0)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_STANDARD_REQUIRED True)

# Specify the executable
add_executable(OpenGLApplication
    src/Camera.cpp
    src/IndexBuffer.cpp
    src/OpenGLApplication.cpp
    src/Renderer.cpp
    src/Shader.cpp
    src/Texture.cpp
    src/Utility.cpp
    src/VertexArray.cpp
    src/VertexBuffer.cpp
    vendor/stb_image/stb_image.cpp
    tests/Test.cpp
    tests/TestClearColor.cpp
    tests/TestTexture2D.cpp
    tests/Test3D.cpp
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_demo.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/imgui_impl_glfw.cpp
    vendor/imgui/imgui_impl_opengl3.cpp
)

# Set directories
set(DEPS_DIR ${CMAKE_SOURCE_DIR}/Dependencies)
set(SHADERS_DIR ${CMAKE_SOURCE_DIR}/shaders)
set(ASSETS_DIR ${CMAKE_SOURCE_DIR}/assets)
set(BUILD_DIR ${CMAKE_SOURCE_DIR}/build) 

# Include directories
target_include_directories(OpenGLApplication PRIVATE 
    ${DEPS_DIR}/GLFW/include
    ${DEPS_DIR}/GLEW/include
    ${DEPS_DIR}/glm
    ${CMAKE_SOURCE_DIR}/vendor/imgui
    ${CMAKE_SOURCE_DIR}/vendor/stb_image
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)

# Add definitions for static GLEW
add_definitions(-DGLEW_STATIC)

# Link directories and libraries for 32-bit and 64-bit configurations
if(CMAKE_SIZEOF_VOID_P EQUAL 8)  # 64-bit
    target_link_libraries(OpenGLApplication PRIVATE 
        ${DEPS_DIR}/GLFW/lib-vc2015/glfw3.lib
        opengl32
        User32
        Gdi32
        Shell32
        ${DEPS_DIR}/GLEW/lib/Release/x64/glew32s.lib
    )
else()  # 32-bit
    target_link_libraries(OpenGLApplication PRIVATE 
        ${DEPS_DIR}/GLFW/lib-vc2015/glfw3.lib
        opengl32
        User32
        Gdi32
        Shell32
        ${DEPS_DIR}/GLEW/lib/Release/Win32/glew32s.lib
    )
endif()

# Ensure the proper startup project in Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLApplication)

# Ensure shaders and assets directories are copied correctly after build
add_custom_command(TARGET OpenGLApplication POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${SHADERS_DIR}" "${BUILD_DIR}/shaders"
)

add_custom_command(TARGET OpenGLApplication POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${ASSETS_DIR}" "${BUILD_DIR}/assets"
)