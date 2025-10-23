cmake_minimum_required(VERSION 3.28)
project(ZeusEngineCore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(assimp REQUIRED)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/include/glfw glfw)

add_library(ZeusEngineCore STATIC
        include/ZeusEngineCore/ModelImporter.h
        include/ZeusEngineCore/EventDispatcher.h)

target_compile_features(ZeusEngineCore PUBLIC cxx_std_23)


file(GLOB_RECURSE ZEUSENGINECORE_SOURCES
        src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/glad/src/glad.c
        ${CMAKE_CURRENT_SOURCE_DIR}/include/tinyfiledialogs/tinyfiledialogs.c
)

target_sources(ZeusEngineCore PRIVATE ${ZEUSENGINECORE_SOURCES} )

target_include_directories(ZeusEngineCore
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/entt
        ${CMAKE_CURRENT_SOURCE_DIR}/include/glad/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/glm
        ${CMAKE_CURRENT_SOURCE_DIR}/include/tinyfiledialogs
)

target_link_libraries(ZeusEngineCore
        PUBLIC
        glfw
        assimp::assimp
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
