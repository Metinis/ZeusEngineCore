#pragma once
#include <optional>
#include "Image.h"

namespace ZEN::VKAPI {
    class CommandBlock;
    struct Bitmap;
    struct ImageCreateInfo;
    class SampledImage {
    public:
        SampledImage(const ImageCreateInfo &createInfo, CommandBlock commandBlock,
                     Bitmap const &bitmap);

        const Image &Get();

    private:
        std::optional<Image> m_SampledImage{};
    };
}