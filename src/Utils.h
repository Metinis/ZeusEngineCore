#pragma once
#include <string>
#include <filesystem>
#include <glm/glm.hpp>
#include "ZeusEngineCore/Material.h"
#include "ZeusEngineCore/IMesh.h"
#include <optional>
#include <vulkan/vulkan.hpp>
#include <bit>

std::string ReadFile(const std::filesystem::path& filePath);

std::vector<std::uint32_t> ToSpirV(const std::filesystem::path& filePath);

void RequireSuccess(vk::Result const result, char const* errorMsg);

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
template <typename T>
[[nodiscard]] constexpr auto ToByteArray(T const& t) {
    return std::bit_cast<std::array<std::byte, sizeof(T)>>(t);
}
template <typename T>
[[nodiscard]] constexpr auto ToByteSpan(const std::vector<T>& vec) {
    return std::span<const std::byte>(
            reinterpret_cast<const std::byte*>(vec.data()),
            vec.size() * sizeof(T)
    );
}



