#pragma once
#include <memory>

namespace ZEN {
    enum class eRendererAPI;
    struct WindowHandle;

    class IRendererBackend {
    public:
        static std::unique_ptr<IRendererBackend> Create(eRendererAPI api, WindowHandle handle);
        virtual ~IRendererBackend() = default;
        [[nodiscard]] virtual eRendererAPI GetAPI() const = 0;
    };

}