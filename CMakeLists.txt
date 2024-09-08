cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(OpenGLApplication VERSION 1.0)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)

# Set directories
set(DEPS_DIR ${CMAKE_SOURCE_DIR}/Dependencies)
set(SHADERS_DIR ${CMAKE_SOURCE_DIR}/shaders)
set(ASSETS_DIR ${CMAKE_SOURCE_DIR}/assets)

# Glob all .cpp and .h files directly in src and tests directories
file(GLOB_RECURSE SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/src/*.h)
file(GLOB_RECURSE TEST_FILES ${CMAKE_SOURCE_DIR}/tests/*.cpp ${CMAKE_SOURCE_DIR}/tests/*.h)

# Vendor files (you may also glob these if desired)
set(VENDOR_FILES
    vendor/stb_image/stb_image.cpp
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_demo.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/imgui_impl_glfw.cpp
    vendor/imgui/imgui_impl_opengl3.cpp
)

# Create source groups for better organization in Visual Studio
source_group("src" FILES ${SRC_FILES})
source_group("tests" FILES ${TEST_FILES})
source_group("vendor" FILES ${VENDOR_FILES})

# Specify the executable
add_executable(OpenGLApplication ${SRC_FILES} ${TEST_FILES} ${VENDOR_FILES})

# Include directories
target_include_directories(OpenGLApplication PRIVATE 
    ${DEPS_DIR}/GLFW/include
    ${DEPS_DIR}/GLEW/include
    ${DEPS_DIR}/glm
    ${DEPS_DIR}/assimp_related/include
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
        ${DEPS_DIR}/assimp_related/lib/assimp-vc143-mt.lib
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