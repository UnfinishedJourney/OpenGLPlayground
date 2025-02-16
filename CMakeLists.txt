cmake_minimum_required(VERSION 3.25)
project(OpenGLApplication VERSION 1.0)

# ==============================================================================
# Options for Compiler and Code Analysis
# ==============================================================================
option(USE_CLANG "Use Clang compiler instead of the default compiler" OFF)
option(USE_CLANG_TIDY "Enable clang-tidy for code analysis" OFF)

if(USE_CLANG)
    # Force Clang for C and C++ if enabled.
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
endif()

# Set C++ Standard (modern CMake way)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable clang-tidy if requested
if(USE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy clang-tidy.exe)
    if(CLANG_TIDY_EXE)
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=*")
    else()
        message(WARNING "clang-tidy not found. Skipping code analysis.")
    endif()
endif()

# ==============================================================================
# Directories and Source Files
# ==============================================================================
set(DEPS_DIR "${CMAKE_SOURCE_DIR}/deps/src")
set(SHADERS_DIR "${CMAKE_SOURCE_DIR}/shaders")
set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")

# Collect main source and test files
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.h"
)
file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/tests/*.cpp"
    "${CMAKE_SOURCE_DIR}/tests/*.h"
)

# Vendor files (e.g. ImGui, GLAD, stb_image)
set(VENDOR_FILES
    "${DEPS_DIR}/imgui/imgui.cpp"
    "${DEPS_DIR}/imgui/imgui_draw.cpp"
    "${DEPS_DIR}/imgui/imgui_demo.cpp"
    "${DEPS_DIR}/imgui/imgui_tables.cpp"
    "${DEPS_DIR}/imgui/imgui_widgets.cpp"
    "${DEPS_DIR}/imgui/backends/imgui_impl_glfw.cpp"
    "${DEPS_DIR}/imgui/backends/imgui_impl_opengl3.cpp"
    "${DEPS_DIR}/GLAD/src/glad.c"
    "${DEPS_DIR}/stb_image/stb_image.cpp"
)

# ==============================================================================
# Group Sources for IDEs (Visual Studio)
# ==============================================================================
function(group_sources base_dir)
    foreach(file IN ITEMS ${ARGN})
        get_filename_component(abs_file "${file}" ABSOLUTE)
        file(RELATIVE_PATH relative_file "${base_dir}" "${abs_file}")
        # Replace "/" with "\" for Windows IDE grouping.
        string(REPLACE "/" "\\" group_name "${relative_file}")
        get_filename_component(group_name "${group_name}" PATH)
        if(group_name STREQUAL "")
            get_filename_component(dir_name "${base_dir}" NAME)
            set(group_name "${dir_name}")
        endif()
        source_group("${group_name}" FILES "${abs_file}")
    endforeach()
endfunction()

group_sources("${CMAKE_SOURCE_DIR}/src" ${SRC_FILES})
group_sources("${CMAKE_SOURCE_DIR}/tests" ${TEST_FILES})
group_sources("${DEPS_DIR}" ${VENDOR_FILES})

# ==============================================================================
# Create Executable
# ==============================================================================
add_executable(OpenGLApplication
    ${SRC_FILES}
    ${TEST_FILES}
    ${VENDOR_FILES}
)

# ==============================================================================
# Include Directories for Main Target
# ==============================================================================
target_include_directories(OpenGLApplication PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/tests"
    "${DEPS_DIR}/GLAD/include"
    "${DEPS_DIR}/imgui"
    "${DEPS_DIR}/imgui/backends"
    "${DEPS_DIR}/stb_image"
)

# ==============================================================================
# EasyProfiler Integration
# ==============================================================================
# (Note: For a robust solution, consider using find_package or CMake options.)
set(EASY_PROFILER_INCLUDE_DIR "C:/Program Files (x86)/easy_profiler/include")
set(EASY_PROFILER_LIB_DIR "C:/Program Files (x86)/easy_profiler/lib")
set(EASY_PROFILER_DLL "C:/Program Files (x86)/easy_profiler/bin/easy_profiler.dll")

target_include_directories(OpenGLApplication PRIVATE "${EASY_PROFILER_INCLUDE_DIR}")
target_link_libraries(OpenGLApplication PRIVATE "${EASY_PROFILER_LIB_DIR}/easy_profiler.lib")
add_custom_command(TARGET OpenGLApplication POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${EASY_PROFILER_DLL}"
            $<TARGET_FILE_DIR:OpenGLApplication>
)

# ==============================================================================
# External Header-only Libraries
# ==============================================================================
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE "${DEPS_DIR}/json/include")

add_library(spdlog INTERFACE)
target_include_directories(spdlog INTERFACE "${DEPS_DIR}/spdlog/include")

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE "${DEPS_DIR}/glm")

# ==============================================================================
# GLFW Integration
# ==============================================================================
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
if(WIN32)
    set(GLFW_BUILD_WIN32 ON CACHE BOOL "" FORCE)
endif()
add_subdirectory("${DEPS_DIR}/glfw")
set_property(TARGET glfw PROPERTY FOLDER "ThirdPartyLibraries")
target_include_directories(OpenGLApplication PRIVATE "${DEPS_DIR}/glfw/include")
target_link_libraries(OpenGLApplication PRIVATE glfw)

# ==============================================================================
# Assimp Integration
# ==============================================================================
set(ASSIMP_BUILD_ZLIB ON     CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON      CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF   CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF       CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON  CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF    CACHE BOOL "" FORCE)
add_subdirectory("${DEPS_DIR}/assimp")
set_property(TARGET assimp PROPERTY FOLDER "ThirdPartyLibraries")
target_include_directories(OpenGLApplication PRIVATE "${DEPS_DIR}/assimp/include")
target_link_libraries(OpenGLApplication PRIVATE assimp)

# ==============================================================================
# Meshoptimizer Integration
# ==============================================================================
set(MESHOPT_BUILD_DEMO OFF    CACHE BOOL "" FORCE)
set(MESHOPT_BUILD_TOOLS OFF   CACHE BOOL "" FORCE)
set(MESHOPT_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory("${DEPS_DIR}/meshoptimizer")
set_property(TARGET meshoptimizer PROPERTY FOLDER "ThirdPartyLibraries")
target_include_directories(OpenGLApplication PRIVATE "${DEPS_DIR}/meshoptimizer/src")
target_link_libraries(OpenGLApplication PRIVATE meshoptimizer)

# ==============================================================================
# Link Header-only and Platform Libraries
# ==============================================================================
target_link_libraries(OpenGLApplication PRIVATE
    nlohmann_json
    spdlog
    glm
    opengl32
)

if(MSVC)
    target_link_libraries(OpenGLApplication PRIVATE
        user32.lib
        gdi32.lib
        shell32.lib
    )
    # Suppress warnings for Assimp and Meshoptimizer
    target_compile_options(assimp PRIVATE /w)
    target_compile_options(meshoptimizer PRIVATE /w)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    # Enable UTF-8 and suppress specific warnings
    target_compile_options(OpenGLApplication PRIVATE /utf-8 /wd4005)
endif()

# ==============================================================================
# Preprocessor Definitions
# ==============================================================================
target_compile_definitions(OpenGLApplication PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)

# ==============================================================================
# Set Output Directories and Visual Studio Properties
# ==============================================================================
set_target_properties(OpenGLApplication PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT OpenGLApplication)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Group external dependencies under ThirdPartyLibraries in IDEs
set_property(TARGET nlohmann_json  PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET spdlog         PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET glm            PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET assimp         PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET meshoptimizer  PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET glfw           PROPERTY FOLDER "ThirdPartyLibraries")

if(TARGET zlibstatic)
    set_property(TARGET zlibstatic PROPERTY FOLDER "ThirdPartyLibraries")
endif()

if(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs)
    set_property(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTY FOLDER "ThirdPartyLibraries")
endif()

set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakePredefinedTargets")