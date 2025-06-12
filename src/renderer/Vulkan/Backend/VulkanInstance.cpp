#include "VulkanInstance.h"
VulkanInstance::VulkanInstance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions)
{

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
    appInfo.apiVersion = VK_API_VERSION_1_0;

    vk::InstanceCreateInfo createInfo{};
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
    createInfo.ppEnabledLayerNames = layers.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(finalExtensions.size());
    createInfo.ppEnabledExtensionNames = finalExtensions.data();
    createInfo.flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;

    m_Instance = vk::createInstanceUnique(createInfo);
}

const bool VulkanInstance::CheckValidationLayerSupport(const std::vector<const char*>& layers)
{
    const auto availableLayers = vk::enumerateInstanceLayerProperties();
    for (const char* layerName : layers) {
        bool layerFound = false;
        for (const auto& availableLayerName : availableLayers) {
            if (strcmp(layerName, availableLayerName.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            return false;
        }
    }
    return true;
}

