#include "VulkanInstance.h"
#include <string_view>
#include <ranges>
#include <algorithm>
VulkanInstance::VulkanInstance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions)
{
    m_ApiVersion = VK_API_VERSION_1_3;
    std::vector<const char*> finalExtensions = extensions;
    if (!layers.empty()) {
        //only add if we have validation layers
        finalExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        if(!CheckValidationLayerSupport(layers))
            throw std::runtime_error("validation layers requested, but not available!");
    }

    vk::ApplicationInfo appInfo{};
    appInfo.pApplicationName = "ZeusEditor";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "ZeusEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = m_ApiVersion;

    vk::InstanceCreateInfo createInfo{};
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
    createInfo.ppEnabledLayerNames = layers.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(finalExtensions.size());
    createInfo.ppEnabledExtensionNames = finalExtensions.data();
#ifdef __APPLE__
    createInfo.flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif

    m_Instance = vk::createInstanceUnique(createInfo);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_Instance);
}

const bool VulkanInstance::CheckValidationLayerSupport(const std::vector<const char*>& layers)
{
    const auto availableLayers = vk::enumerateInstanceLayerProperties();
    //create a list of names from available layers
    auto availableNames = availableLayers | std::views::transform([](const auto& layer) {
        return std::string_view{ layer.layerName };
        });
    //for each layer name, check that its in available layer names
    return std::ranges::all_of(layers, [&](const char* layer) {
        return std::ranges::find(availableNames, std::string_view{ layer }) != availableNames.end();
        });
}

