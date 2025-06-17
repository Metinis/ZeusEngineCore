#pragma once
#include <string>
#include <filesystem>
#include <glm/glm.hpp>
#include "ZeusEngineCore/Material.h"
#include "ZeusEngineCore/IMesh.h"
#include <optional>
#include <vulkan/vulkan.hpp>

using DispatchLoaderDynamic = vk::detail::DispatchLoaderDynamic;

std::string readFile(const std::filesystem::path& filePath);

void requireSuccess(vk::Result const result, char const* errorMsg);

struct WindowHandle {
    void* nativeWindowHandle;


};
struct RendererInitInfo {
    std::optional<WindowHandle> windowHandle;

};
struct RenderCommand {
    glm::mat4 transform;
    std::shared_ptr<Material> material;
    std::shared_ptr<IMesh> mesh;
};