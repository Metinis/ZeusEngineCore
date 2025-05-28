cmake_minimum_required(VERSION 3.28)
project(ZeusEngineCore)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add glad source directory
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
)

# Engine public headers
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
)

# Create static library
add_library(ZeusEngineCore STATIC ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(ZeusEngineCore PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include             # ZeusEngineCore public headers
)

target_include_directories(ZeusEngineCore PRIVATE
        ${CMAKE_SOURCE_DIR}/include/glm                 # glm math
)

# Platform-specific dependencies
if (APPLE)
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)

    target_link_libraries(ZeusEngineCore PRIVATE
            ${COCOA_LIBRARY}
            ${IOKIT_LIBRARY}
            ${COREVIDEO_LIBRARY}
    )
endif()

# Vulkan and glad
find_package(Vulkan REQUIRED)
target_link_libraries(ZeusEngineCore PRIVATE Vulkan::Vulkan)
target_link_libraries(ZeusEngineCore PUBLIC glad glfw)