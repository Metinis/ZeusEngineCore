#pragma once
#include <string>
#include <filesystem>
#include <glm/glm.hpp>
#include "ZeusEngineCore/Material.h"
#include "ZeusEngineCore/IMesh.h"

std::string readFile(const std::filesystem::path& filePath);

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