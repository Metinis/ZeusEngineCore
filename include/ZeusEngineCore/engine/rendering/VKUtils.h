#pragma once

#define VK_CHECK(x)                                                     \
do {                                                                \
VkResult err = x;                                               \
if (err) {                                                      \
const std::string errMsg = "Detected Vulkan error: " + std::string(string_VkResult(err));                                                            \
throw std::runtime_error(errMsg);\
}                                                               \
} while (0)