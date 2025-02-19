cmake_minimum_required(VERSION 3.25)

# =====================================================================
# Compiler Options and Clang-Tidy Configuration
# =====================================================================
option(USE_CLANG "Use Clang compiler instead of the default compiler" OFF)
option(USE_CLANG_TIDY "Enable clang-tidy for code analysis" OFF)

if(USE_CLANG)
    # Set Clang as the C and C++ compiler
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
endif()

project(OpenGLApplication VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable clang-tidy if the option is activated
if(USE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy clang-tidy.exe)
    if(CLANG_TIDY_EXE)
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=*")
    else()
        message(WARNING "clang-tidy not found. Skipping code analysis.")
    endif()
endif()

# =====================================================================
# Directory Settings
# =====================================================================
set(DEPS_DIR ${CMAKE_SOURCE_DIR}/deps/src)
set(SHADERS_DIR ${CMAKE_SOURCE_DIR}/shaders)
set(ASSETS_DIR ${CMAKE_SOURCE_DIR}/assets)

# =====================================================================
# Source Files
# =====================================================================

# Recursively find all .cpp and .h files in src and tests directories
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/*.h
)
file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/tests/*.cpp
    ${CMAKE_SOURCE_DIR}/tests/*.h
)

# Vendor library files
# (Note: Instead of a physical stb_image.cpp, we generate one below.)
set(VENDOR_FILES
    # ImGui source files
    ${DEPS_DIR}/imgui/imgui.cpp
    ${DEPS_DIR}/imgui/imgui_draw.cpp
    ${DEPS_DIR}/imgui/imgui_demo.cpp
    ${DEPS_DIR}/imgui/imgui_tables.cpp
    ${DEPS_DIR}/imgui/imgui_widgets.cpp
    ${DEPS_DIR}/imgui/backends/imgui_impl_glfw.cpp
    ${DEPS_DIR}/imgui/backends/imgui_impl_opengl3.cpp

    # GLAD source file (handled manually)
    ${DEPS_DIR}/GLAD/src/glad.c
)

# Generate a temporary implementation file for stb_image
set(STB_IMPL_FILE "${CMAKE_BINARY_DIR}/stb_image_impl.cpp")
file(WRITE "${STB_IMPL_FILE}" 
"#define STB_IMAGE_IMPLEMENTATION\n\
#define STB_IMAGE_WRITE_IMPLEMENTATION\n\
#define STB_IMAGE_RESIZE_IMPLEMENTATION\n\
#include \"stb_image.h\"\n\
#include \"stb_image_write.h\"\n\
#include \"stb_image_resize2.h\"\n"
)
list(APPEND VENDOR_FILES "${STB_IMPL_FILE}")

# =====================================================================
# Function to Group Sources by Directory
# =====================================================================
function(group_sources base_dir)
    foreach(file IN ITEMS ${ARGN})
        get_filename_component(abs_file ${file} ABSOLUTE)
        file(RELATIVE_PATH relative_file ${base_dir} ${abs_file})
        string(REPLACE "/" "\\" group_name ${relative_file})
        get_filename_component(group_name "${group_name}" PATH)

        # If group_name is empty, set it to the base directory name
        if(group_name STREQUAL "")
            get_filename_component(dir_name ${base_dir} NAME)
            set(group_name ${dir_name})
        endif()

        source_group("${group_name}" FILES "${abs_file}")
    endforeach()
endfunction()

group_sources(${CMAKE_SOURCE_DIR}/src ${SRC_FILES})
group_sources(${CMAKE_SOURCE_DIR}/tests ${TEST_FILES})
group_sources(${DEPS_DIR} ${VENDOR_FILES})

# =====================================================================
# Executable Target
# =====================================================================
add_executable(OpenGLApplication
    ${SRC_FILES}
    ${TEST_FILES}
    ${VENDOR_FILES}
)

# =====================================================================
# EasyProfiler Integration
# =====================================================================
# Specify paths to EasyProfiler
set(EASY_PROFILER_INCLUDE_DIR "C:/Program Files (x86)/easy_profiler/include")
set(EASY_PROFILER_LIB_DIR "C:/Program Files (x86)/easy_profiler/lib")
set(EASY_PROFILER_DLL "C:/Program Files (x86)/easy_profiler/bin/easy_profiler.dll")

# Add include directory for EasyProfiler
target_include_directories(OpenGLApplication PRIVATE ${EASY_PROFILER_INCLUDE_DIR})

# Link with EasyProfiler library
target_link_libraries(OpenGLApplication PRIVATE "${EASY_PROFILER_LIB_DIR}/easy_profiler.lib")

# Automatically copy EasyProfiler DLL to the build directory
add_custom_command(TARGET OpenGLApplication POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${EASY_PROFILER_DLL}"
    $<TARGET_FILE_DIR:OpenGLApplication>
)

# =====================================================================
# Include Directories
# =====================================================================
target_include_directories(OpenGLApplication PRIVATE
    ${DEPS_DIR}/GLAD/include
    ${DEPS_DIR}/imgui
    ${DEPS_DIR}/imgui/backends
    ${DEPS_DIR}/stb_image    # Include stb_image directory
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)

# =====================================================================
# nlohmann/json Integration
# =====================================================================
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE ${DEPS_DIR}/json/include)

# =====================================================================
# spdlog Integration
# =====================================================================
add_library(spdlog INTERFACE)
target_include_directories(spdlog INTERFACE ${DEPS_DIR}/spdlog/include)

# =====================================================================
# glm Integration
# =====================================================================
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE ${DEPS_DIR}/glm)

# =====================================================================
# GLFW Integration
# =====================================================================
# GLFW configuration options
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

if(WIN32)
    set(GLFW_BUILD_WIN32 ON CACHE BOOL "" FORCE)  # Enable Win32 support on Windows
endif()

# Add subdirectory for GLFW
add_subdirectory(${DEPS_DIR}/glfw)

# Group GLFW under 'ThirdPartyLibraries' folder in Visual Studio
set_property(TARGET glfw PROPERTY FOLDER "ThirdPartyLibraries")

# Include GLFW headers
target_include_directories(OpenGLApplication PRIVATE ${DEPS_DIR}/glfw/include)

# Link against GLFW
target_link_libraries(OpenGLApplication PRIVATE glfw)

# =====================================================================
# Assimp Integration
# =====================================================================
# Assimp configuration options
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Add subdirectory for Assimp
add_subdirectory(${DEPS_DIR}/assimp)

# Group Assimp under 'ThirdPartyLibraries' folder in Visual Studio
set_property(TARGET assimp PROPERTY FOLDER "ThirdPartyLibraries")

# Include Assimp headers
target_include_directories(OpenGLApplication PRIVATE ${DEPS_DIR}/assimp/include)

# Link against Assimp
target_link_libraries(OpenGLApplication PRIVATE assimp)

# =====================================================================
# Meshoptimizer Integration
# =====================================================================
# Meshoptimizer configuration options
set(MESHOPT_BUILD_DEMO OFF CACHE BOOL "" FORCE)
set(MESHOPT_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(MESHOPT_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Add subdirectory for Meshoptimizer
add_subdirectory(${DEPS_DIR}/meshoptimizer)

# Group Meshoptimizer under 'ThirdPartyLibraries' folder in Visual Studio
set_property(TARGET meshoptimizer PROPERTY FOLDER "ThirdPartyLibraries")

# Include Meshoptimizer headers
target_include_directories(OpenGLApplication PRIVATE ${DEPS_DIR}/meshoptimizer/src)

# Link against Meshoptimizer
target_link_libraries(OpenGLApplication PRIVATE meshoptimizer)

# =====================================================================
# Linking Libraries
# =====================================================================
# Header-only libraries
target_link_libraries(OpenGLApplication PRIVATE
    nlohmann_json
    spdlog
    glm
)

# Link other libraries
target_link_libraries(OpenGLApplication PRIVATE
    glfw
    opengl32
    "${EASY_PROFILER_LIB_DIR}/easy_profiler.lib"
)

# Platform-specific libraries
if(MSVC)
    target_link_libraries(OpenGLApplication PRIVATE
        user32.lib
        gdi32.lib
        shell32.lib
    )
endif()

# =====================================================================
# Additional Definitions and Compiler Options
# =====================================================================
# Define IMGUI_IMPL_OPENGL_LOADER_GLAD for ImGui
target_compile_definitions(OpenGLApplication PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)

# Suppress warnings and set compiler options
if(MSVC)
    # Suppress warnings from Assimp and Meshoptimizer
    target_compile_options(assimp PRIVATE /w)
    target_compile_options(meshoptimizer PRIVATE /w)
    # Suppress CRT security warnings
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    target_compile_options(OpenGLApplication PRIVATE /utf-8)  # Enable UTF-8 encoding
    target_compile_options(OpenGLApplication PRIVATE /wd4005) # Suppress 'APIENTRY' macro redefinition warning
endif()

# =====================================================================
# Set Output Directories
# =====================================================================
set_target_properties(OpenGLApplication PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# =====================================================================
# Visual Studio Startup Project
# =====================================================================
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLApplication)

# =====================================================================
# Organize External Libraries in Visual Studio Solution
# =====================================================================
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(TARGET nlohmann_json PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET spdlog PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET glm PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET assimp PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET meshoptimizer PROPERTY FOLDER "ThirdPartyLibraries")
if(TARGET zlibstatic)
    set_property(TARGET zlibstatic PROPERTY FOLDER "ThirdPartyLibraries")
endif()
if(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs)
    set_property(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTY FOLDER "ThirdPartyLibraries")
endif()
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakePredefinedTargets")