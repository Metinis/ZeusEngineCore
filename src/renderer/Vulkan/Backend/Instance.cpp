#include "Instance.h"
#include <string_view>
#include <ranges>
#include <algorithm>
#include <ranges>
#include "GLFW/glfw3.h"
#include <array>

using namespace ZEN::VKAPI;

constexpr std::array layers_v {
#ifndef NDEBUG
        "VK_LAYER_KHRONOS_validation",
#endif
        "VK_LAYER_KHRONOS_shader_object"
};

constexpr std::array extensions_v{
#ifndef NDEBUG
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
#ifdef __APPLE__
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
#endif
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
};

Instance::Instance()
{
    VULKAN_HPP_DEFAULT_DISPATCHER.init();
    if (!CheckLayerSupport())
        throw std::runtime_error("Instance Layers requested, but not available!");

    //glfw uses C style array passing hence we retrieve count
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> finalExtensions;
    finalExtensions.reserve(glfwExtensionCount + extensions_v.size());
    finalExtensions.insert(finalExtensions.begin(), glfwExtensions, glfwExtensions + glfwExtensionCount);
    finalExtensions.insert(finalExtensions.end(), extensions_v.begin(), extensions_v.end());


    if (!CheckExtensionSupport(finalExtensions))
        throw std::runtime_error("Instance Extensions requested, but not available!");

    //todo fetch newest vk version
    m_ApiVersion = VK_API_VERSION_1_3;

    vk::ApplicationInfo appInfo{};
    appInfo.setPApplicationName("ZeusEngine")
    .setApiVersion(m_ApiVersion);

    vk::InstanceCreateInfo createInfo{};
    createInfo.setPApplicationInfo(&appInfo)
    .setPEnabledExtensionNames(finalExtensions)
    .setPEnabledLayerNames(layers_v);
#ifdef __APPLE__
    createInfo.flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif

    m_Instance = vk::createInstanceUnique(createInfo);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_Instance);
}

const bool Instance::CheckLayerSupport()
{
    const auto availableLayers = vk::enumerateInstanceLayerProperties();
    //create a list of names from available layers
    auto availableNames = availableLayers | std::views::transform([](const auto& layer) {
        return std::string_view{ layer.layerName };
        });
    //for each layer name, check that its in available layer names
    return std::ranges::all_of(layers_v, [&](const char* layer) {
        return std::ranges::find(availableNames, std::string_view{ layer }) != availableNames.end();
        });
}

const bool Instance::CheckExtensionSupport(const std::vector<const char*>& finalExtensions) {
    const auto availableExtensions = vk::enumerateInstanceExtensionProperties();
    //create a list of names from available extensions
    auto availableNames = availableExtensions | std::views::transform([](const auto& extension) {
        return std::string_view{ extension.extensionName };
    });
    //for each extensions name, check that its in available extension names
    return std::ranges::all_of(finalExtensions, [&](const char* extension) {
        return std::ranges::find(availableNames, std::string_view{ extension }) != availableNames.end();
    });
}