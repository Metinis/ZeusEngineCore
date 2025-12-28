#pragma once
#include <ZeusEngineCore/API.h>
#include <assimp/scene.h>
#include <ZeusEngineCore/UUID.h>
#include <ZeusEngineCore/AssetTypes.h>

namespace ZEN {

    using AssetID = UUID;

    struct UniformBuffer {
        uint32_t uboID{};
    };

    struct GPUTexture {
        uint32_t drawableID{0};
        TextureType type{};

    };
    struct GPUShader {
        uint32_t drawableID{};
    };
    struct GPUMesh {
        uint32_t drawableID{};
        size_t indexCount{};
        int instanceCount{1};
    };

    using GPUVariant = std::variant<
        GPUMesh,
        GPUTexture,
        GPUShader
    //Add to this for more asset types
    >;
    using GPUMap = std::unordered_map<AssetID, GPUVariant>;

    struct MeshData;
    struct Material;
    struct MaterialRaw;
    class IResourceManager {
    public:
        template<typename T, typename F>
        static void withResource(std::unordered_map<uint32_t, T>& resources, uint32_t id, F&& func) {
            auto it = resources.find(id);
            if (it == resources.end()) {
                std::cerr << "Resource not found: " << id << "\n";
                return;
            }
            func(it->second);
        }
        template<typename T>
        std::optional<GPUVariant> create(AssetID id, T asset) {
            if constexpr (std::is_same_v<T, TextureData>) {
                //todo change this to be more generic
                if(asset.type == Texture2D && asset.absPath) {
                    auto ret = GPUTexture {
                        .drawableID = createTexture(asset.path, true),
                        .type = asset.type,
                    };
                    m_Mappings.emplace(id, ret);
                    return ret;
                }
                else if(asset.type == Texture2D && !asset.absPath) {
                    auto ret = GPUTexture {
                        .drawableID = createTexture(asset.path, false),
                        .type = asset.type,
                    };
                    m_Mappings.emplace(id, ret);
                    return ret;
                }
                else if(asset.type == Texture2DAssimp) {
                    auto ret = GPUTexture {
                        .drawableID = createTextureAssimp(*asset.aiTex),
                        .type = asset.type,
                    };
                    m_Mappings.emplace(id, ret);
                    return ret;
                }
                else if(asset.type == CubemapHDR && asset.mip) {
                    auto ret = GPUTexture {
                        .drawableID = createCubeMapTextureHDRMip(
                        asset.dimensions.x, asset.dimensions.y),
                        .type = asset.type,
                    };
                    m_Mappings.emplace(id, ret);
                    return ret;
                }
                else if(asset.type == CubemapHDR) {
                    auto ret = GPUTexture {
                        .drawableID = createCubeMapTextureHDR(
                        asset.dimensions.x, asset.dimensions.y),
                        .type = asset.type,
                    };
                    m_Mappings.emplace(id, ret);
                    return ret;
                }
                else if(asset.type == Cubemap) {
                    auto ret = GPUTexture {
                        .drawableID = createCubeMapTexture(asset.path),
                        .type = asset.type,
                    };
                    m_Mappings.emplace(id, ret);
                    return ret;
                }
                else if(asset.type == HDR) {
                    auto ret = GPUTexture {
                        .drawableID = createHDRTexture(asset.path),
                        .type = asset.type,
                    };
                    m_Mappings.emplace(id, ret);
                    return ret;
                }
                else if(asset.type == Prefilter) {
                    auto ret = GPUTexture {
                        .drawableID = createPrefilterMap(asset.dimensions.x, asset.dimensions.y),
                        .type = asset.type,
                    };
                    m_Mappings.emplace(id, ret);
                    return ret;
                }
                else if(asset.type == BRDF) {
                    auto ret = GPUTexture {
                        .drawableID = createBRDFLUTTexture(asset.dimensions.x, asset.dimensions.y),
                        .type = asset.type,
                    };
                    m_Mappings.emplace(id, ret);
                    return ret;
                }
            }
            if constexpr (std::is_same_v<T, MeshData>) {
                auto ret = GPUMesh {
                    .drawableID = createMeshDrawable(asset),
                    .indexCount = asset.indices.size(),
                    .instanceCount = 1,
                };
                m_Mappings.emplace(id, ret);
                return ret;
            }
            if constexpr (std::is_same_v<T, ShaderData>) {
                auto ret = GPUShader {
                    .drawableID = createShader(
                        asset.vertPath,
                        asset.fragPath,
                        asset.geoPath),
                };
                m_Mappings.emplace(id, ret);
                return ret;
            }
        }
        template <typename T>
        T* get(AssetID id) {
            auto it = m_Mappings.find(id);
            if (it == m_Mappings.end()) {
                if constexpr (std::is_same_v<T, GPUTexture>) {
                  static GPUTexture defaultTexture{.drawableID = 0};
                return &defaultTexture;
                }
                std::cout<<"GPU Resource not found! returning nullptr: "<<id;
                return nullptr;
            }
            return std::get_if<T>(&it->second);
        }
        bool has(AssetID id) {
            auto it = m_Mappings.find(id);
            if (it == m_Mappings.end()) {
                return false;
            }
            return true;
        }

        virtual ~IResourceManager() = default;

        virtual uint32_t createMeshDrawable(const MeshData& mesh) = 0;
        virtual void bindMeshDrawable(uint32_t drawableID) = 0;
        virtual void deleteMeshDrawable(uint32_t drawableID) = 0;

        virtual uint32_t createShader(const std::string& vertexPath, const std::string& fragPath,
            const std::string& geoPath) = 0;
        virtual void bindShader(uint32_t shaderID) = 0;
        virtual void deleteShader(uint32_t shaderID) = 0;

        virtual uint32_t createUBO(uint32_t binding) = 0;
        virtual void bindUBO(uint32_t uboID) = 0;
        virtual void writeToUBO(uint32_t uboID, std::span<const std::byte> bytes) = 0;
        virtual void deleteUBO(uint32_t uboID) = 0;

        virtual uint32_t createTexture(const std::string& texturePath, bool isAbsPath) = 0;
        virtual uint32_t createHDRTexture(const std::string& texturePath) = 0;
        virtual uint32_t createTextureAssimp(const aiTexture& aiTex) = 0;
        virtual uint32_t createBRDFLUTTexture(uint32_t width, uint32_t height) = 0;
        virtual void genMipMapCubeMap(uint32_t textureID) = 0;
        virtual void setFBOCubeMapTexture(uint32_t binding, uint32_t textureID, unsigned int mip) = 0;
        virtual void setFBOTexture2D(uint32_t binding, uint32_t textureID, unsigned int mip) = 0;
        virtual void bindTexture(uint32_t textureID, uint32_t binding) = 0;
        virtual void deleteTexture(uint32_t textureID) = 0;

        virtual void bindMaterial(const MaterialRaw& material) = 0;

        virtual uint32_t createCubeMapTexture(const std::string& texturePath) = 0;
        virtual uint32_t createCubeMapTextureHDRMip(uint32_t width, uint32_t height) = 0;
        virtual uint32_t createCubeMapTextureHDR(uint32_t width, uint32_t height) = 0;
        virtual uint32_t createPrefilterMap(uint32_t width, uint32_t height) = 0;
        virtual void bindCubeMapTexture(uint32_t textureID, uint32_t binding) = 0;

        virtual uint32_t createFBO() = 0;
        virtual void bindFBO(uint32_t fboID) = 0;

        virtual uint32_t createColorTex(int width, int height) = 0;
        virtual uint32_t getTexture(uint32_t textureID) = 0;
        virtual uint32_t createDepthStencilBuffer(int width, int height) = 0;
        virtual uint32_t createDepthBuffer(int width, int height) = 0;
        virtual void updateDepthBufferDimensions(int width, int height) = 0;
        virtual void bindDepthBuffer(uint32_t bufferID) = 0;
        virtual void deleteDepthBuffer(uint32_t bufferID) = 0;

        virtual void pushFloat(uint32_t shaderID, const std::string& name, float value) = 0;

        std::string fullPath(const std::string& path) {
            return m_ResourceRoot + path;
        }

        static std::unique_ptr<IResourceManager> create();
    protected:
        std::string m_ResourceRoot{};
        GPUMap m_Mappings{};
    };
}
