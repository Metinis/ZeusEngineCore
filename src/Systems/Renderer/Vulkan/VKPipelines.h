#pragma once
#include <vulkan/vulkan.h>

namespace ZEN {
    class VKPipelines {
    public:
        static bool loadShaderModule(std::string_view filePath, VkDevice device, VkShaderModule* outShaderModule);
    };
}

