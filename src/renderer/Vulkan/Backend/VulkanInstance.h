#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>
#include <cstdint>
class VulkanInstance {
public:
    VulkanInstance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions);
    ~VulkanInstance() = default;
    const bool CheckLayerSupport(const std::vector<const char*>& layers);
    [[nodiscard]] vk::Instance Get() const { return *m_Instance; }
    vk::UniqueInstance& GetUnique() { return m_Instance; }
    std::uint32_t GetApiVersion() const { return m_ApiVersion; }

private:
    vk::UniqueInstance m_Instance;
    std::uint32_t m_ApiVersion{};
};