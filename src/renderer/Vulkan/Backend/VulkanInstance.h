#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>
class VulkanInstance {
public:
    VulkanInstance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions);
    ~VulkanInstance() = default;
    const bool CheckValidationLayerSupport(const std::vector<const char*>& layers);
    [[nodiscard]] vk::Instance Get() const { return *m_Instance; }
    vk::UniqueInstance& GetUnique() { return m_Instance; }

private:
    vk::UniqueInstance m_Instance;
};