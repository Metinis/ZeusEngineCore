#include "APIBackend.h"
#include "ZeusEngineCore/IRendererAPI.h"
#include "Mesh.h"
#include "Texture.h"

using namespace ZEN::OGLAPI;
ZEN::eRendererAPI ZEN::OGLAPI::APIBackend::GetAPI() const {
    return ZEN::eRendererAPI::OpenGL;
}

MeshInfo APIBackend::GetMeshInfo() const {
    return MeshInfo{};
}

TextureInfo APIBackend::GetTextureInfo() const {
    return TextureInfo{};
}

ShaderInfo APIBackend::GetShaderInfo() const {
    return ShaderInfo{};
}

BackendInfo APIBackend::GetInfo() const {
    return BackendInfo{};
}
