#pragma once
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>
#include <cstdint>
#include <glm/vec2.hpp>
#include <functional>
#include <memory>
#include "ZeusEngineCore/InfoVariants.h"

namespace ZEN::VKAPI {
    struct ImageHandle {
        VmaAllocator allocator{};
        VmaAllocation allocation{};
        vk::Image image{};
        vk::Extent2D extent{};
        vk::Format format{};
        std::uint32_t levels{};
    };

    struct ImageCreateInfo {
        VmaAllocator allocator;
        std::uint32_t queueFamily;
        std::shared_ptr<std::function<void(DeferredHandle)>> destroyCallback;
    };

    struct Bitmap {
        std::span<std::byte const> bytes{};
        glm::ivec2 size{};
        const void* owner = nullptr;
    };

    class Image {
    public:
        Image(ImageCreateInfo const &createInfo, vk::ImageUsageFlags usage,
              std::uint32_t levels, vk::Format format, vk::Extent2D extent);

        // No copy
        Image(const Image &) = delete;

        Image &operator=(const Image &) = delete;

        // Move
        Image(Image &&other) noexcept {
            *this = std::move(other);
        }

        Image &operator=(Image &&other) noexcept;

        ~Image() {
            Destroy();
        }

        [[nodiscard]] vk::Image Get() const { return m_Handle.image; }

        [[nodiscard]] std::uint32_t GetLevels() const { return m_Handle.levels; }

        [[nodiscard]] vk::Format GetFormat() const { return m_Handle.format; }

    private:
        void Destroy();

        std::function<void(DeferredHandle)> m_DestroyCallback;
        ImageHandle m_Handle{};

    };
}