cmake_minimum_required(VERSION 3.10)

# =====================================================================
# Опции для выбора компилятора и включения clang-tidy
# =====================================================================

option(USE_CLANG "Использовать компилятор Clang вместо текущего компилятора" OFF)
option(USE_CLANG_TIDY "Включить clang-tidy для анализа кода" OFF)

if(USE_CLANG)
    # Устанавливаем Clang как компилятор C и C++
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
endif()

# =====================================================================
# Основная конфигурация проекта
# =====================================================================

# Название проекта и версия
project(OpenGLApplication VERSION 1.0)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Включение clang-tidy, если опция активирована
if(USE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy clang-tidy.exe)
    if(CLANG_TIDY_EXE)
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=*")
    else()
        message(WARNING "clang-tidy не найден. Пропуск анализа кода.")
    endif()
endif()

# Установка директорий
set(DEPS_DIR ${CMAKE_SOURCE_DIR}/Dependencies)
set(SHADERS_DIR ${CMAKE_SOURCE_DIR}/shaders)
set(ASSETS_DIR ${CMAKE_SOURCE_DIR}/assets)

# Поиск всех .cpp и .h файлов в структуре
file(GLOB_RECURSE SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/src/*.h)
file(GLOB_RECURSE TEST_FILES ${CMAKE_SOURCE_DIR}/tests/*.cpp ${CMAKE_SOURCE_DIR}/tests/*.h)

# Файлы сторонних библиотек
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

# Функция для группировки исходников по директориям
function(group_sources base_dir)
    foreach(file IN ITEMS ${ARGN})
        get_filename_component(abs_file ${file} ABSOLUTE)
        file(RELATIVE_PATH relative_file ${base_dir} ${abs_file})
        string(REPLACE "/" "\\" group_name ${relative_file})
        get_filename_component(group_name "${group_name}" PATH)

        # Если group_name пустой, устанавливаем имя базовой директории
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

# Указание исполняемого файла
add_executable(OpenGLApplication ${SRC_FILES} ${TEST_FILES} ${VENDOR_FILES})

target_include_directories(OpenGLApplication PRIVATE 
    ${DEPS_DIR}/GLFW/include
    ${DEPS_DIR}/GLAD/include
    ${DEPS_DIR}/json/include
    ${DEPS_DIR}/spdlog/include
    ${DEPS_DIR}/glm
    ${DEPS_DIR}/assimp_related/include
    ${CMAKE_SOURCE_DIR}/vendor/imgui
    ${CMAKE_SOURCE_DIR}/vendor/stb_image
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)

# Линковка библиотек для 32-битных и 64-битных конфигураций
if(CMAKE_SIZEOF_VOID_P EQUAL 8)  # 64-бит
    target_link_libraries(OpenGLApplication PRIVATE 
        ${DEPS_DIR}/GLFW/lib-vc2015/glfw3.lib
        opengl32
        User32
        Gdi32
        Shell32
        ${DEPS_DIR}/assimp_related/lib/assimp-vc143-mt.lib
        ${DEPS_DIR}/spdlog/lib/spdlog.lib
    )
else()  # 32-бит
    target_link_libraries(OpenGLApplication PRIVATE 
        ${DEPS_DIR}/GLFW/lib-vc2015/glfw3.lib
        opengl32
        User32
        Gdi32
        Shell32
    )
endif()

# Установка правильного стартап-проекта в Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLApplication)