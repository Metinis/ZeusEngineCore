#pragma once
#include <string>
#include <filesystem>
#include <glm/glm.hpp>
#include <optional>
#include <vulkan/vulkan.hpp>
#include <bit>
struct GLFWwindow;

namespace ZEN {

    std::string ReadFile(const std::filesystem::path &filePath);

    std::vector<std::uint32_t> ToSpirV(const std::filesystem::path &filePath);

    void RequireSuccess(vk::Result const result, char const *errorMsg);

    struct WindowHandle {
        //struct exists if different windowing system was to be added
        //use std::variant in that case
        GLFWwindow *nativeWindowHandle;
    };
    struct RendererInitInfo {
        WindowHandle windowHandle;
    };

    template<typename T>
    [[nodiscard]] constexpr auto ToByteArray(T const &t) {
        return std::bit_cast<std::array<std::byte, sizeof(T)>>(t);
    }

    template<typename T>
    [[nodiscard]] constexpr auto ToByteSpan(const std::vector<T> &vec) {
        return std::span<const std::byte>(
                reinterpret_cast<const std::byte *>(vec.data()),
                vec.size() * sizeof(T)
        );
    }
}



