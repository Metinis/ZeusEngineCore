
#include "GLResourceManager.h"
#define GLFW_INCLUDE_NONE
#include <iostream>
#include <fstream>
#include <sstream>
#include "GLFW/glfw3.h"
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

ZEN::GLResourceManager::~GLResourceManager() {
    for (auto& [id, drawable] : m_Drawables) {
        deleteMeshDrawable(id);
    }
    for (auto& [id, drawable] : m_Shaders) {
        deleteShader(id);
    }
    for (auto& [id, drawable] : m_UBOs) {
        deleteUBO(id);
    }
}


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
    withResource(m_Drawables, drawableID, [](GLDrawable& d) {
        glBindVertexArray(d.vao);
    });
}


void ZEN::GLResourceManager::deleteMeshDrawable(uint32_t drawableID) {
    withResource(m_Drawables, drawableID, [](GLDrawable& d) {
        glDeleteBuffers(1, &d.vbo);
        glDeleteBuffers(1, &d.ebo);
        glDeleteVertexArrays(1, &d.vao);
    });
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

    //todo make this dynamic
    GLuint viewBlockIndex = glGetUniformBlockIndex(program, "View");
    glUniformBlockBinding(program, viewBlockIndex, 0);

    GLuint instanceBlockIndex = glGetUniformBlockIndex(program, "Instances");
    glUniformBlockBinding(program, instanceBlockIndex, 1);

    GLuint globalBlockIndex = glGetUniformBlockIndex(program, "Globals");
    glUniformBlockBinding(program, globalBlockIndex, 2);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    uint32_t id = nextShaderID++;
    m_Shaders[id] = GLShader{.programID = program};

    return id;
}

void ZEN::GLResourceManager::bindShader(uint32_t shaderID) {
    withResource(m_Shaders, shaderID, [](GLShader& s) {
        glUseProgram(s.programID);
    });
}

void ZEN::GLResourceManager::deleteShader(uint32_t shaderID) {
    withResource(m_Shaders, shaderID, [](GLShader& s) {
        glDeleteProgram(s.programID);
    });
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
    withResource(m_UBOs, uboID, [bytes](GLUniform& u) {
        glBindBuffer(GL_UNIFORM_BUFFER, u.bufferHandle);
        if(u.size < bytes.size()){
            u.size = bytes.size();
            glBufferData(GL_UNIFORM_BUFFER, u.size, nullptr, GL_DYNAMIC_DRAW);
        }
        glBufferSubData(GL_UNIFORM_BUFFER, 0, bytes.size(), bytes.data());
    });
}

void ZEN::GLResourceManager::bindUBO(uint32_t uboID) {
    withResource(m_UBOs, uboID, [](GLUniform& u) {
        glBindBuffer(GL_UNIFORM_BUFFER, u.bufferHandle);
        glBindBufferBase(GL_UNIFORM_BUFFER, u.bufferBinding, u.bufferHandle);
    });

}

void ZEN::GLResourceManager::deleteUBO(uint32_t uboID) {
    withResource(m_UBOs, uboID, [](GLUniform& u) {
        glDeleteBuffers(1, &u.bufferHandle);
    });
}

uint32_t ZEN::GLResourceManager::createTexture(std::string_view texturePath) {
    int texWidth, texHeight, texChannels;
    stbi_set_flip_vertically_on_load(true);
    stbi_uc* pixels = stbi_load(texturePath.data(), &texWidth,
        &texHeight, &texChannels, STBI_rgb_alpha);
    if(!pixels){
        throw std::runtime_error("Invalid image!");
    }

    GLTexture texture{};
    glGenTextures(1, &texture.textureID);
    glBindTexture(GL_TEXTURE_2D, texture.textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(pixels);

    m_Textures[nextTextureID++] = texture;

    return texture.textureID;
}

void ZEN::GLResourceManager::bindTexture(uint32_t textureID) {
    withResource(m_Textures, textureID, [](GLTexture& t) {
        glBindTexture(GL_TEXTURE_2D, t.textureID);
    });
}

void ZEN::GLResourceManager::deleteTexture(uint32_t textureID) {
    withResource(m_Textures, textureID, [](GLTexture& t) {
        glDeleteTextures(1, &t.textureID);
    });
}



