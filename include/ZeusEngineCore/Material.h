#pragma once
#include <memory>

#include "ZeusEngineCore/IShader.h"

class Material {
public:
    explicit Material(const std::shared_ptr<IShader>& shader);

    std::shared_ptr<IShader>& GetShader();

    void SetColor(const std::string& name, const glm::vec4& color);

    void SetFloat(const std::string& name, float value);

    //void SetTexture(const std::string& name, std::sha)

    //glm::vec4& ColorRef() { return m_Color; }  // allow ImGui to edit directly

    glm::vec4& ColorRef(const std::string& name);

    void Bind();

    void Unbind() const;

private:
    std::shared_ptr<IShader> m_Shader;
    std::unordered_map<std::string, glm::vec4> m_Colors;
    std::unordered_map<std::string, float> m_Floats;
    //std::unordered_map<std::string, std::shared_ptr<Texture>> m_Textures;
};
