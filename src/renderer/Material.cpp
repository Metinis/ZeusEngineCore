#include "ZeusEngineCore/Material.h"

Material::Material(const std::shared_ptr<IShader>& shader) {
    m_Shader = shader;
}
std::shared_ptr<IShader>& Material::GetShader() {
    return m_Shader;
}
void Material::Bind() {
    m_Shader->Bind();

    for (const auto& [name, color] : m_Colors)
        m_Shader->SetUniformVec4(name, color);

    for (const auto& [name, value] : m_Floats)
        m_Shader->SetUniformFloat(name, value);

    /*int slot = 0;
    for (const auto& [name, texture] : m_Textures) {
        texture->Bind(slot);
        m_Shader->SetUniformInt(name, slot);
        slot++;
    }*/
}
glm::vec4& Material::ColorRef(const std::string& name) {
    return m_Colors[name];
}

void Material::Unbind() const {
    m_Shader->Unbind();
}

void Material::SetColor(const std::string &name, const glm::vec4 &color) {
    m_Colors[name] = color;
}
void Material::SetFloat(const std::string &name, const float value) {
    m_Floats[name] = value;
}



