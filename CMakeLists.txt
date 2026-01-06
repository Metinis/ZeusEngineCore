cmake_minimum_required(VERSION 3.16)
project(ZeusEngineCore LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/include/glfw glfw)

add_library(ZeusEngineCore SHARED)

target_compile_features(ZeusEngineCore PUBLIC cxx_std_23)

add_library(imgui STATIC)

target_include_directories(imgui PUBLIC
        include/imgui
        include/imgui/backends
)
target_compile_definitions(imgui PUBLIC IMGUI_DEFINE_MATH_OPERATORS)

target_sources(imgui PRIVATE
        include/imgui/imgui.cpp
        include/imgui/imgui_draw.cpp
        include/imgui/imgui_tables.cpp
        include/imgui/imgui_widgets.cpp
        include/imgui/imgui_demo.cpp
        include/imgui/backends/imgui_impl_glfw.cpp
        include/imgui/backends/imgui_impl_opengl3.cpp
)

target_link_libraries(imgui
        PRIVATE
        glfw
)

add_library(ImGuizmo STATIC)

target_include_directories(ImGuizmo PUBLIC
    include/ImGuizmo
        include/imgui
)

target_sources(ImGuizmo PRIVATE
    include/ImGuizmo/ImGuizmo.cpp
)
target_link_libraries(ImGuizmo
        PRIVATE
        imgui
)


file(GLOB_RECURSE ZEUSENGINECORE_SOURCES
        src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/glad/src/glad.c
        ${CMAKE_CURRENT_SOURCE_DIR}/include/tinyfiledialogs/tinyfiledialogs.c
)

file(GLOB_RECURSE ZEUSENGINECORE_HEADERS include/*.h include/*.hpp)
target_sources(ZeusEngineCore PRIVATE
        ${ZEUSENGINECORE_SOURCES}
        ${ZEUSENGINECORE_HEADERS}
)

target_sources(ZeusEngineCore PRIVATE ${ZEUSENGINECORE_SOURCES} )

target_precompile_headers(ZeusEngineCore
        PUBLIC
        "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/include/ZeusEngineCore/ZENPCH.h>"
)

target_include_directories(ZeusEngineCore
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/entt
        ${CMAKE_CURRENT_SOURCE_DIR}/include/glad/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/glm
        ${CMAKE_CURRENT_SOURCE_DIR}/include/tinyfiledialogs
)
include(FetchContent)

FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG v6.0.2
)
FetchContent_MakeAvailable(assimp)

FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG master
)
FetchContent_MakeAvailable(yaml-cpp)

target_link_libraries(ZeusEngineCore
        PRIVATE
        glfw
        PUBLIC
        assimp::assimp
        imgui
        ImGuizmo
        yaml-cpp::yaml-cpp
)

if(APPLE)
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)

    target_link_libraries(ZeusEngineCore PRIVATE
            ${COCOA_LIBRARY}
            ${IOKIT_LIBRARY}
            ${COREVIDEO_LIBRARY}
    )
endif()
