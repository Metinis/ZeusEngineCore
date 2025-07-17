#pragma once
#include <memory>
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>


namespace ZEN {
    class IShader;
    class ITexture;
    class Material {
    public:
        explicit Material(const std::shared_ptr<IShader> &shader);

        std::shared_ptr<IShader> &GetShader();

        void SetColor(const std::string &name, const glm::vec4 &color);

        void SetFloat(const std::string &name, float value);

        void SetTexture(const std::shared_ptr<ITexture>& texture);

        //glm::vec4& ColorRef() { return m_Color; }  // allow ImGui to edit directly

        glm::vec4 &ColorRef(const std::string &name);

        void Bind();

        void Unbind() const;

    private:
        std::shared_ptr<IShader> m_Shader;
        std::shared_ptr<ITexture> m_Texture;
        std::unordered_map<std::string, glm::vec4> m_Colors;
        std::unordered_map<std::string, float> m_Floats;
    };
}
