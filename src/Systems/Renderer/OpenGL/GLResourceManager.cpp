#include "GLResourceManager.h"
#define GLFW_INCLUDE_NONE
#include <iostream>
#include <fstream>
#include <sstream>
#include "GLFW/glfw3.h"
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#include <ZeusEngineCore/ModelLibrary.h>

constexpr std::array uboBindings{
    "View",
    "Instances",
    "Globals",
    "Material"
};
constexpr std::array textureBindings{
    "u_DiffuseMap",
    "u_SpecularMap"
};

ZEN::GLResourceManager::GLResourceManager() {
    unsigned char whitePixel[4] = { 255, 255, 255, 255 };
    GLTexture tex{};
    glGenTextures(1, &tex.textureID);
    glBindTexture(GL_TEXTURE_2D, tex.textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_Textures[0] = tex;
}
ZEN::GLResourceManager::~GLResourceManager() {
    for (auto &[id, drawable]: m_Drawables) {
        deleteMeshDrawable(id);
    }
    for (auto &[id, drawable]: m_Shaders) {
        deleteShader(id);
    }
    for (auto &[id, drawable]: m_UBOs) {
        deleteUBO(id);
    }
}


uint32_t ZEN::GLResourceManager::createMeshDrawable(const Mesh &mesh) {
    GLDrawable drawable{};
    glGenVertexArrays(1, &drawable.vao);
    glGenBuffers(1, &drawable.vbo);
    glGenBuffers(1, &drawable.ebo);

    glBindVertexArray(drawable.vao);

    glBindBuffer(GL_ARRAY_BUFFER, drawable.vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawable.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(uint32_t), mesh.indices.data(),
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, Position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, Normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, TexCoords));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, Color));

    glBindVertexArray(0);

    m_Drawables[nextDrawableID] = drawable;

    return nextDrawableID++;
}

void ZEN::GLResourceManager::bindMeshDrawable(uint32_t drawableID) {
    withResource(m_Drawables, drawableID, [](GLDrawable &d) {
        glBindVertexArray(d.vao);
    });
}


void ZEN::GLResourceManager::deleteMeshDrawable(uint32_t drawableID) {
    withResource(m_Drawables, drawableID, [](GLDrawable &d) {
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
    const char *vSrc = vertexSrc.c_str();
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
    const char *fSrc = fragSrc.c_str();
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

    //ubo bindings
    for (uint32_t i{0}; i < uboBindings.size(); ++i) {
        GLuint blockIndex = glGetUniformBlockIndex(program, uboBindings[i]);
        if (blockIndex != GL_INVALID_INDEX) {
            glUniformBlockBinding(program, blockIndex, i);
        }
    }

    //texture bindings
    /*glUseProgram(program);
    for (int i{0}; i < textureBindings.size(); ++i) {
        GLint location = glGetUniformLocation(program, textureBindings[i]);
        if (location != GL_INVALID_INDEX) {
            glUniform1i(location, i);
        }
    }*/

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    uint32_t id = nextShaderID++;
    m_Shaders[id] = GLShader{.programID = program};

    return id;
}

void ZEN::GLResourceManager::bindShader(uint32_t shaderID) {
    withResource(m_Shaders, shaderID, [](GLShader &s) {
        glUseProgram(s.programID);
    });
}

void ZEN::GLResourceManager::deleteShader(uint32_t shaderID) {
    withResource(m_Shaders, shaderID, [](GLShader &s) {
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
    withResource(m_UBOs, uboID, [bytes](GLUniform &u) {
        glBindBuffer(GL_UNIFORM_BUFFER, u.bufferHandle);
        if (u.size < bytes.size()) {
            u.size = bytes.size();
            glBufferData(GL_UNIFORM_BUFFER, u.size, nullptr, GL_DYNAMIC_DRAW);
        }
        glBufferSubData(GL_UNIFORM_BUFFER, 0, bytes.size(), bytes.data());
    });
}

void ZEN::GLResourceManager::bindUBO(uint32_t uboID) {
    withResource(m_UBOs, uboID, [](GLUniform &u) {
        glBindBuffer(GL_UNIFORM_BUFFER, u.bufferHandle);
        glBindBufferBase(GL_UNIFORM_BUFFER, u.bufferBinding, u.bufferHandle);
    });
}

void ZEN::GLResourceManager::deleteUBO(uint32_t uboID) {
    withResource(m_UBOs, uboID, [](GLUniform &u) {
        glDeleteBuffers(1, &u.bufferHandle);
    });
}

uint32_t ZEN::GLResourceManager::createTexture(std::string_view texturePath) {
    int texWidth, texHeight, texChannels;
    stbi_set_flip_vertically_on_load(true);
    stbi_uc *pixels = stbi_load(texturePath.data(), &texWidth,
                                &texHeight, &texChannels, STBI_rgb_alpha);
    if (!pixels) {
        std::cout<<"Invalid Image! Assigning default texture.."<<"\n";
        return 0;
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

    m_Textures[nextTextureID] = texture;

    return nextTextureID++;
}

uint32_t ZEN::GLResourceManager::createTextureAssimp(const aiTexture& aiTex) {
    unsigned char* imageData;
    int width, height, channels;
    bool compressed = false;
    if (aiTex.mHeight == 0) {
        // Compressed (PNG/JPG) in memory
        imageData = stbi_load_from_memory(
            reinterpret_cast<unsigned char*>(aiTex.pcData),
            aiTex.mWidth,
            &width, &height, &channels, STBI_rgb_alpha
        );
        compressed = true;
        if (!imageData || width == 0 || height == 0) {
            std::cerr << "Failed to load compressed embedded texture!\n";
            return 0;
        }
    } else {
        // Raw RGBA
        width = aiTex.mWidth;
        height = aiTex.mHeight;
        channels = 4;
        imageData = reinterpret_cast<unsigned char*>(aiTex.pcData);
        if (!aiTex.pcData || width == 0 || height == 0) {
            std::cerr << "Invalid raw embedded texture!\n";
            return 0;
        }
    }

    GLTexture texture{};
    glGenTextures(1, &texture.textureID);
    glBindTexture(GL_TEXTURE_2D, texture.textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, imageData);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    if(compressed) {
        stbi_image_free(imageData);
    }

    m_Textures[nextTextureID] = texture;
    return nextTextureID++;
}

void ZEN::GLResourceManager::bindTexture(uint32_t textureID, uint32_t binding) {
    withResource(m_Textures, textureID, [binding](GLTexture &t) {
        glActiveTexture(GL_TEXTURE0 + binding);
        glBindTexture(GL_TEXTURE_2D, t.textureID);
    });
}

void ZEN::GLResourceManager::deleteTexture(uint32_t textureID) {
    withResource(m_Textures, textureID, [](GLTexture &t) {
        glDeleteTextures(1, &t.textureID);
    });
}

void ZEN::GLResourceManager::bindMaterial(const Material &material) {
    bindShader(material.shaderID);

    //Bind diffuse textures first
    for (size_t i = 0; i < material.textureIDs.size(); ++i) {
        bindTexture(material.textureIDs[i], i);

        withResource(m_Shaders, material.shaderID, [&](GLShader& s) {
            std::string name = "u_DiffuseMap[" + std::to_string(i) + "]";
            GLint loc = glGetUniformLocation(s.programID, name.c_str());
            if (loc != -1) glUniform1i(loc, i);
        });
    }

    //Bind specular textures next
    for (size_t i = 0; i < material.specularTexIDs.size(); ++i) {
        bindTexture(material.specularTexIDs[i], material.textureIDs.size() + i);

        withResource(m_Shaders, material.shaderID, [&](GLShader& s) {
            std::string name = "u_SpecularMap[" + std::to_string(i) + "]";
            GLint loc = glGetUniformLocation(s.programID, name.c_str());
            if (loc != -1) glUniform1i(loc, material.textureIDs.size() + i);
        });
    }
}

constexpr std::array<std::string, 6> cubeSides {
    "right.jpg",
    "left.jpg",
    "top.jpg",
    "bottom.jpg",
    "front.jpg",
    "back.jpg"

};

uint32_t ZEN::GLResourceManager::createCubeMapTexture(const std::string& texturePath) {
    //gen the texture handle
    GLTexture texture{};
    glGenTextures(1, &texture.textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture.textureID);
    m_Textures[nextTextureID] = texture;

    //load the faces
    int texWidth, texHeight, texChannels;
    stbi_set_flip_vertically_on_load(false);
    for (unsigned int i = 0; i < cubeSides.size(); i++) {
        std::string facePath = texturePath;
        facePath += cubeSides[i];

        stbi_uc *pixels = stbi_load(facePath.c_str(), &texWidth,
                                    &texHeight, &texChannels, STBI_rgb);
        if (!pixels) {
            throw std::runtime_error("Invalid image!");
        }
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,0, GL_RGB,
            texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        stbi_image_free(pixels);
    }



    //return the id in resources
    return nextTextureID++;
}


void ZEN::GLResourceManager::bindCubeMapTexture(uint32_t textureID) {
    withResource(m_Textures, textureID, [](GLTexture &t) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, t.textureID);
    });
}

uint32_t ZEN::GLResourceManager::createFBO() {
    GLFBO fbo{};
    glGenFramebuffers(1, &fbo.handle);
    m_FBOs[nextFBOID] = fbo;
    return nextFBOID++;
}

void ZEN::GLResourceManager::bindFBO(uint32_t fboID) {
    if(fboID == 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }
    withResource(m_FBOs, fboID, [](GLFBO &f) {
        glBindFramebuffer(GL_FRAMEBUFFER, f.handle);
    });
}


uint32_t ZEN::GLResourceManager::createColorTex(int width, int height) {
    GLTexture tex{};
    glGenTextures(1, &tex.textureID);
    glBindTexture(GL_TEXTURE_2D, tex.textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, tex.textureID, 0);
    m_Textures[nextTextureID] = tex;
    return nextTextureID++;
}

uint32_t ZEN::GLResourceManager::getTexture(uint32_t textureID) {
    uint32_t ret = 0;
    withResource(m_Textures, textureID, [&ret](GLTexture &t) {
        ret = t.textureID;
    });
    return ret;
}

uint32_t ZEN::GLResourceManager::createDepthBuffer(int width, int height) {
    GLDepthBuffer depthBuffer{};
    glGenRenderbuffers(1, &depthBuffer.handle);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer.handle);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, depthBuffer.handle);
    m_DepthBuffers[nextDepthBufferID] = depthBuffer;
    return nextDepthBufferID++;
}

void ZEN::GLResourceManager::deleteDepthBuffer(uint32_t bufferID) {
    withResource(m_DepthBuffers, bufferID, [](GLDepthBuffer &b) {
        glDeleteRenderbuffers(1, &b.handle);
    });

}
