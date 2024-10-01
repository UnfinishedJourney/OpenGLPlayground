cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(OpenGLApplication VERSION 1.0)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)

# Set directories
set(DEPS_DIR ${CMAKE_SOURCE_DIR}/Dependencies)
set(SHADERS_DIR ${CMAKE_SOURCE_DIR}/shaders)
set(ASSETS_DIR ${CMAKE_SOURCE_DIR}/assets)

# Glob all .cpp and .h files in the new structure
file(GLOB_RECURSE SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/src/*.h)
file(GLOB_RECURSE TEST_FILES ${CMAKE_SOURCE_DIR}/tests/*.cpp ${CMAKE_SOURCE_DIR}/tests/*.h)

# Vendor files
set(VENDOR_FILES
    ${CMAKE_SOURCE_DIR}/vendor/stb_image/stb_image.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_impl_opengl3.cpp
    ${DEPS_DIR}/GLAD/src/glad.c
)

# Function to group sources based on their directory structure
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
group_sources(${CMAKE_SOURCE_DIR}/vendor ${VENDOR_FILES})

# Specify the executable
add_executable(OpenGLApplication ${SRC_FILES} ${TEST_FILES} ${VENDOR_FILES})

target_include_directories(OpenGLApplication PRIVATE 
    ${DEPS_DIR}/GLFW/include
    ${DEPS_DIR}/GLAD/include
    ${DEPS_DIR}/json/include
    ${DEPS_DIR}/glm
    ${DEPS_DIR}/assimp_related/include
    ${CMAKE_SOURCE_DIR}/vendor/imgui
    ${CMAKE_SOURCE_DIR}/vendor/stb_image
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)

# Link directories and libraries for 32-bit and 64-bit configurations
if(CMAKE_SIZEOF_VOID_P EQUAL 8)  # 64-bit
    target_link_libraries(OpenGLApplication PRIVATE 
        ${DEPS_DIR}/GLFW/lib-vc2015/glfw3.lib
        opengl32
        User32
        Gdi32
        Shell32
        ${DEPS_DIR}/assimp_related/lib/assimp-vc143-mt.lib
    )
else()  # 32-bit
    target_link_libraries(OpenGLApplication PRIVATE 
        ${DEPS_DIR}/GLFW/lib-vc2015/glfw3.lib
        opengl32
        User32
        Gdi32
        Shell32
    )
endif()

# Ensure the proper startup project in Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLApplication)