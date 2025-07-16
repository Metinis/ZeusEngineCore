#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <glm/mat4x4.hpp>
#include <variant>
#include <string_view>
#include <memory>

namespace ZEN {
    class IRendererBackend;
    class IRendererAPI;

    class IShader {
    public:
        virtual ~IShader() = default;

        virtual void Bind() const = 0;

        virtual void Unbind() const = 0;

        virtual void SetUniformInt(const std::string &name, int value) = 0;

        virtual void SetUniformMat4(const std::string &name, const glm::mat4 &matrix) = 0;

        virtual void SetUniformFloat(const std::string &name, float value) = 0;

        virtual void SetUniformVec4(const std::string &name, const glm::vec4 &value) = 0;

        bool *GetWireframeFlag() { return &m_IsWireframe; }

        float *GetLineWidth() { return &m_LineWidth; }

        static std::shared_ptr<IShader> Create(IRendererBackend* apiBackend,
                                               IRendererAPI* apiRenderer,
                                               const std::string& vertexPath,
                                               const std::string& fragmentPath);

    protected:
        std::uint32_t m_RendererID;
        bool m_IsWireframe = false;
        std::unordered_map<std::string, int> m_UniformLocationCache;
        float m_LineWidth{1.0f};

        virtual int GetUniformLocation(const std::string &name) = 0;
    };
}
