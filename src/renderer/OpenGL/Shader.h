#pragma once
#include <string>
#include <unordered_map>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>

#include "ZeusEngineCore/IShader.h"

namespace ZEN::OGLAPI {
    class Shader : public IShader {
    public:
        ~Shader() override;

        void Bind() const override;

        void Unbind() const override;

        void SetUniformInt(const std::string &name, int value) override;

        void SetUniformMat4(const std::string &name, const glm::mat4 &matrix) override;

        void SetUniformFloat(const std::string &name, float value) override;

        void SetUniformVec4(const std::string &name, const glm::vec4 &value) override;


    private:
        uint32_t m_RendererID;
        std::unordered_map<std::string, int> m_UniformLocationCache;

        int GetUniformLocation(const std::string &name) override;
    };
}
