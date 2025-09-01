
#include "GLResourceManager.h"
#define GLFW_INCLUDE_NONE
#include <iostream>
#include <fstream>
#include <sstream>
#include "GLFW/glfw3.h"
#include <glad/glad.h>

uint32_t ZEN::GLResourceManager::createMeshDrawable(const MeshComp &meshComp) {
    GLDrawable drawable{};
    glGenVertexArrays(1, &drawable.vao);
    glGenBuffers(1, &drawable.vbo);
    glGenBuffers(1, &drawable.ebo);

    glBindVertexArray(drawable.vao);

    glBindBuffer(GL_ARRAY_BUFFER, drawable.vbo);
    glBufferData(GL_ARRAY_BUFFER, meshComp.vertices.size() * sizeof(Vertex), meshComp.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawable.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshComp.indices.size() * sizeof(uint32_t), meshComp.indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Color));

    glBindVertexArray(0);

    m_Drawables[nextDrawableID] = drawable;

    return nextDrawableID++;
}

void ZEN::GLResourceManager::bindMeshDrawable(uint32_t drawableID) {
    auto it = m_Drawables.find(drawableID);
    if(it != m_Drawables.end()) {
        glBindVertexArray(it->second.vao);
    }
    else {
        std::cerr << "Drawable not found" << "\n";
    }
}


void ZEN::GLResourceManager::deleteMeshDrawable(uint32_t drawableID) {
    auto it = m_Drawables.find(drawableID);
    if(it != m_Drawables.end()) {
        glDeleteBuffers(1, &it->second.vbo);
        glDeleteBuffers(1, &it->second.ebo);
        glDeleteVertexArrays(1, &it->second.vao);
    }
    else {
        std::cerr << "Drawable not found" << "\n";
    }
}


uint32_t ZEN::GLResourceManager::createShader(std::string_view vertexPath, std::string_view fragPath) {
    auto loadShaderSource = [](std::string_view path) -> std::string {
        std::ifstream file(path.data());
        if (!file.is_open()) {
            std::cerr << "Failed to open shader file: " << path << "\n";
            return "";
        }
        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    };

    std::string vertexSrc = loadShaderSource(vertexPath);
    std::string fragSrc = loadShaderSource(fragPath);

    if (vertexSrc.empty() || fragSrc.empty()) {
        return 0; // 0 = invalid shader ID
    }

    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vSrc = vertexSrc.c_str();
    glShaderSource(vertexShader, 1, &vSrc, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compilation failed:\n" << infoLog << "\n";
        glDeleteShader(vertexShader);
        return 0;
    }

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fSrc = fragSrc.c_str();
    glShaderSource(fragmentShader, 1, &fSrc, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compilation failed:\n" << infoLog << "\n";
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }

    // Link program
    uint32_t program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << "\n";
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(program);
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    uint32_t id = nextShaderID++;
    m_Shaders[id] = GLShader{program};

    return id;
}

void ZEN::GLResourceManager::bindShader(uint32_t shaderID) {
    auto it = m_Shaders.find(shaderID);
    if(it != m_Shaders.end()) {
        glUseProgram(it->second.programID);
    }
    else {
        std::cerr << "Shader not found" << "\n";
    }
}

void ZEN::GLResourceManager::deleteShader(uint32_t shaderID) {
    auto it = m_Shaders.find(shaderID);
    if(it != m_Shaders.end()) {
        glDeleteProgram(it->second.programID);
    }
    else {
        std::cerr << "Shader not found" << "\n";
    }
}

uint32_t ZEN::GLResourceManager::createUBO(uint32_t binding) {
    GLUniform uboHandle{.bufferBinding = binding, .size = 1};
    glGenBuffers(1, &uboHandle.bufferHandle);
    glBindBuffer(GL_UNIFORM_BUFFER, uboHandle.bufferHandle);
    glBufferData(GL_UNIFORM_BUFFER, uboHandle.size, nullptr, GL_DYNAMIC_DRAW); // allocate memory
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    m_UBOs[nextUBOID] = uboHandle;
    return nextUBOID++;
}

void ZEN::GLResourceManager::writeToUBO(uint32_t uboID, std::span<const std::byte> bytes) {
    auto it = m_UBOs.find(uboID);
    if(it != m_UBOs.end()) {
        glBindBuffer(GL_UNIFORM_BUFFER, it->second.bufferHandle);
        if(it->second.size < bytes.size()){
            it->second.size = bytes.size();
            glBufferData(GL_UNIFORM_BUFFER, it->second.size, nullptr, GL_DYNAMIC_DRAW);
        }
        glBufferSubData(GL_UNIFORM_BUFFER, 0, bytes.size(), bytes.data());
    }
    else {
        std::cerr << "UBO not found" << "\n";
    }
}

void ZEN::GLResourceManager::bindUBO(uint32_t uboID) {
    auto it = m_UBOs.find(uboID);
    if(it != m_UBOs.end()) {
        glBindBuffer(GL_UNIFORM_BUFFER, it->second.bufferHandle);
        glBindBufferBase(GL_UNIFORM_BUFFER, it->second.bufferBinding, it->second.bufferHandle);
    }
    else {
        std::cerr << "UBO not found" << "\n";
    }

}

void ZEN::GLResourceManager::deleteUBO(uint32_t uboID) {

}



