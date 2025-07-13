#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>
#include <cstdint>

namespace ZEN::VKAPI {
    class Instance {
    public:
        Instance();

        ~Instance() = default;

        const bool CheckLayerSupport(); //uses the constexpr in the cpp file

        //uses constexpr in cpp file combined with glfw extensions at runtime, hence passing argument
        const bool CheckExtensionSupport(const std::vector<const char *> &finalExtensions);

        [[nodiscard]] vk::Instance Get() const { return *m_Instance; }

        vk::UniqueInstance &GetUnique() { return m_Instance; }

        std::uint32_t GetApiVersion() const { return m_ApiVersion; }

    private:
        vk::UniqueInstance m_Instance;
        std::uint32_t m_ApiVersion{};
    };
}
