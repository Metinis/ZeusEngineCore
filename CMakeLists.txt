cmake_minimum_required(VERSION 3.28)
project(ZeusEngineCore)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(glad STATIC ${CMAKE_CURRENT_SOURCE_DIR}/include/glad/src/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/glad/include)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/include/glfw)

# Engine source files
set(SOURCES
    src/renderer/IRenderer.cpp
    src/renderer/OpenGL/GLRenderer.cpp
    src/renderer/Vulkan/VKRenderer.cpp
    src/renderer/OpenGL/GLShader.cpp
    src/renderer/Material.cpp
    src/renderer/IMesh.cpp
    src/renderer/OpenGL/GLMesh.cpp
    src/renderer/Window.cpp
    src/renderer/Vulkan/VKMesh.cpp
    src/renderer/ShaderManager.cpp
    src/renderer/IShader.cpp
    src/renderer/MeshManager.cpp
    src/renderer/MaterialManager.cpp
    src/Utils.cpp
)

set(HEADERS
    include/ZeusEngineCore/IRenderer.h
    src/renderer/OpenGL/GLRenderer.h
    src/renderer/Vulkan/VKRenderer.h
    include/ZeusEngineCore/RendererAPI.h
    src/renderer/OpenGL/GLShader.h
    src/renderer/OpenGL/GLMesh.h
    include/ZeusEngineCore/Window.h
    include/ZeusEngineCore/IMesh.h
    include/ZeusEngineCore/Material.h
    src/renderer/Vulkan/VKMesh.h
    include/ZeusEngineCore/ShaderManager.h
    include/ZeusEngineCore/IShader.h
    src/renderer/Vulkan/VKShader.cpp
    src/renderer/Vulkan/VKShader.h
    include/ZeusEngineCore/MeshManager.h
    include/ZeusEngineCore/MaterialManager.h
    src/Utils.h
)

add_library(ZeusEngineCore STATIC ${SOURCES} ${HEADERS})

target_include_directories(ZeusEngineCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(ZeusEngineCore PRIVATE
    ${CMAKE_SOURCE_DIR}/include/glm
)

# Link glad and glfw
target_link_libraries(ZeusEngineCore PUBLIC glad glfw)

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(ZeusEngineCore PRIVATE Vulkan::Vulkan)

# Platform-specific frameworks (macOS)
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
