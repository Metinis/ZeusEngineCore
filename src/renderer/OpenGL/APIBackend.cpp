#include <glm/ext/matrix_clip_space.hpp>
#include "APIBackend.h"
#include "ZeusEngineCore/IRendererAPI.h"
#include "Mesh.h"
#include "Texture.h"
#include "ZeusEngineCore/IDescriptorBuffer.h"
#include "ZeusEngineCore/Vertex.h"
#include "Shader.h"
#include "ZeusEngineCore/InfoVariants.h"
#include <glfw/glfw3.h>

using namespace ZEN::OGLAPI;

APIBackend::APIBackend(const ZEN::WindowHandle& windowHandle) : m_WindowHandle(windowHandle){
}

ZEN::eRendererAPI ZEN::OGLAPI::APIBackend::GetAPI() const {
    return ZEN::eRendererAPI::OpenGL;
}

MeshInfo APIBackend::GetMeshInfo() const {
    return MeshInfo{};
}

TextureInfo APIBackend::GetTextureInfo() const {
    return TextureInfo{};
}

ShaderInfo APIBackend::GetShaderInfo() const {
    return ShaderInfo{};
}

BackendInfo APIBackend::GetInfo() const {
    return BackendInfo{};
}

BufferCreateInfo APIBackend::GetBufferCreateInfo(const ZEN::eDescriptorBufferType type) const {
    return BufferCreateInfo{};
}

glm::mat4 APIBackend::GetPerspectiveMatrix(float fov, float zNear, float zFar) const {
    glm::vec2 framebufferSize = GetFramebufferSize();
    float aspect = framebufferSize.x / framebufferSize.y;
    glm::mat4 projectionMat = glm::perspective(glm::radians(fov), aspect, zNear, zFar);
    return projectionMat;
}

glm::vec2 APIBackend::GetFramebufferSize() const {
    int width;
    int height;
    glfwGetFramebufferSize(m_WindowHandle.nativeWindowHandle, &width, &height);
    return {width, height};
}


