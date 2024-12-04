cmake_minimum_required(VERSION 3.25)

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
set(DEPS_SRC_DIR ${CMAKE_SOURCE_DIR}/deps/src)
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

# =====================================================================
# Ручное подключение easy_profiler
# =====================================================================

# Укажите пути к easy_profiler
set(EASY_PROFILER_INCLUDE_DIR "C:/Program Files (x86)/easy_profiler/include")
set(EASY_PROFILER_LIB_DIR "C:/Program Files (x86)/easy_profiler/lib")
set(EASY_PROFILER_DLL "C:/Program Files (x86)/easy_profiler/bin/easy_profiler.dll")

# Добавление include директории
target_include_directories(OpenGLApplication PRIVATE ${EASY_PROFILER_INCLUDE_DIR})

# Линковка с библиотекой easy_profiler
target_link_libraries(OpenGLApplication PRIVATE "${EASY_PROFILER_LIB_DIR}/easy_profiler.lib")

# Автоматическое копирование easy_profiler.dll в директорию сборки
add_custom_command(TARGET OpenGLApplication POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${EASY_PROFILER_DLL}"
    $<TARGET_FILE_DIR:OpenGLApplication>)

# Дополнительные include директории
target_include_directories(OpenGLApplication PRIVATE 
    ${DEPS_DIR}/GLFW/include
    ${DEPS_DIR}/GLAD/include
    ${DEPS_DIR}/spdlog/include
    ${DEPS_DIR}/GLM
    ${DEPS_DIR}/assimp_related/include
    ${DEPS_SRC_DIR}/meshoptimizer/src        # Include meshoptimizer
    ${CMAKE_SOURCE_DIR}/vendor/imgui
    ${CMAKE_SOURCE_DIR}/vendor/stb_image
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)

# Создание импортированного целевого объекта для nlohmann/json
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE ${DEPS_SRC_DIR}/json/include)

# =====================================================================
# Настройка Assimp для включения только OBJ и GLTF импортёров
# =====================================================================
set(ASSIMP_NO_EXPORT ON CACHE BOOL "Disable export functionality in Assimp" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "Disable Assimp tools" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Disable Assimp tests" FORCE)
set(ASSIMP_INSTALL_PDB OFF CACHE BOOL "Disable installation of PDB files" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "Disable building all importers by default" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "Enable OBJ importer in Assimp" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "Enable GLTF importer in Assimp" FORCE)

# Добавляем поддиректорию для meshoptimizer
add_subdirectory(${DEPS_SRC_DIR}/meshoptimizer)

# Добавляем поддиректорию для Assimp
add_subdirectory(${DEPS_SRC_DIR}/assimp)

# Линковка библиотек для 32-битных и 64-битных конфигураций
# Link header-only libraries outside the conditional
target_link_libraries(OpenGLApplication PRIVATE 
    nlohmann_json
    assimp            # Link Assimp via CMake target
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)  # 64-bit
    target_link_libraries(OpenGLApplication PRIVATE 
        ${DEPS_DIR}/GLFW/lib-vc2015/glfw3.lib
        opengl32
        User32
        Gdi32
        Shell32
        ${DEPS_DIR}/spdlog/lib/spdlog.lib
        meshoptimizer      # Link meshoptimizer
    )
else()  # 32-bit
    target_link_libraries(OpenGLApplication PRIVATE 
        ${DEPS_DIR}/GLFW/lib-vc2015/glfw3.lib
        opengl32
        User32
        Gdi32
        Shell32
        meshoptimizer      # Link meshoptimizer
    )
endif()

# Установка правильного стартап-проекта в Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLApplication)