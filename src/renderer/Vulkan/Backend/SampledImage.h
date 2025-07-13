#pragma once
#include "Image.h"
#include "CommandBlock.h"

namespace ZEN::VKAPI {
    class SampledImage {
    public:
        SampledImage(const ImageCreateInfo &createInfo, CommandBlock commandBlock,
                     Bitmap const &bitmap);

        const Image &Get() { return *m_SampledImage; }

    private:
        std::optional<Image> m_SampledImage;
    };
}