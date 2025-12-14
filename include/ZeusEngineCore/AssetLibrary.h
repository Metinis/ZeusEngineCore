#pragma once
#include "UUID.h"
#include "Vertex.h"
#include "../../src/Systems/Renderer/IResourceManager.h"

namespace ZEN {
    using AssetID = UUID;
    //cant remove these
    const std::unordered_set<std::string> defaultMeshes = {
        "Cube", "Sphere", "Capsule"
    };
    const std::unordered_set<std::string> defaultMaterials = {
        "Default"
    };
    struct Material {
        AssetID shader{0};
        AssetID texture{0};
        AssetID metallicTex{0};
        AssetID roughnessTex{0};
        AssetID normalTex{0};
        AssetID aoTex{0};
        glm::vec3 albedo{1.0f, 1.0f, 1.0f};
        float metallic{1.0f};
        float roughness{1.0f};
        float ao{1.0f};
        bool metal{false};
        bool useAlbedo{false};
        bool useMetallic{false};
        bool useRoughness{false};
        bool useNormal{false};
        bool useAO{false};
    };
    struct MaterialRaw {
        uint32_t shaderID{0};
        uint32_t textureID{0};
        uint32_t metallicTexID{0};
        uint32_t roughnessTexID{0};
        uint32_t normalTexID{0};
        uint32_t aoTexID{0};
        glm::vec3 albedo{1.0f, 1.0f, 1.0f};
        float metallic{1.0f};
        float roughness{1.0f};
        float ao{1.0f};
        bool metal{false};
        bool useAlbedo{false};
        bool useMetallic{false};
        bool useRoughness{false};
        bool useNormal{false};
        bool useAO{false};
    };
    struct MeshDrawable {
        uint32_t drawableID{};
        size_t indexCount{};
        int instanceCount{1};
    };

    struct MeshData {
        std::vector<uint32_t> indices{};
        std::vector<Vertex> vertices{};
    };

    struct TextureData {
        std::string path;
        uint32_t id;
    };
    struct ShaderData {
        std::string vertPath;
        std::string fragPath;
        std::string geoPath;
        uint32_t id;
    };
    class IResourceManager;
    class EventDispatcher;

    using AssetVariant = std::variant<
        MeshData,
        MeshDrawable,
        Material,
        TextureData,
        ShaderData
    //Add to this for more asset types
    >;

    using AssetMap = std::unordered_map<AssetID, AssetVariant>;

    class AssetLibrary {
    public:
        explicit AssetLibrary(IResourceManager* resourceManager, const std::string& resourceRoot);

        template<typename T>
        AssetID createAsset(T&& asset) {
            AssetID id;
            if constexpr (std::is_same_v<T, TextureData>) {
                asset.id = m_ResourceManager->createTexture(asset.path, false);
            }
            if constexpr (std::is_same_v<T, ShaderData>) {
                asset.id = m_ResourceManager->createShader(asset.vertPath, asset.fragPath, asset.geoPath);
            }
            m_AssetMap.emplace(id, std::forward<T>(asset));
            return id;
        }

        template<typename T>
        void addAsset(AssetID id, T&& asset) {
            m_AssetMap[id] = std::forward<T>(asset);
        }

        template <typename T>
        T* get(AssetID id) {
            auto it = m_AssetMap.find(id);
            if (it == m_AssetMap.end()) {
                if constexpr (std::is_same_v<T, TextureData>) {
                    static TextureData defaultTexture{.id = 0};
                    return &defaultTexture;
                }
                std::cout<<"Asset not found! returning nullptr: "<<id;
                return nullptr;
            }
            return std::get_if<T>(&it->second);
        }

        template<typename T>
        std::vector<T*> getAllOfType() {
            std::vector<T*> ret;
            for (auto& [id, asset] : m_AssetMap) {
                if (auto ptr = std::get_if<T>(&asset)) {
                    ret.push_back(ptr);
                }
            }
            return ret;
        }

        template<typename T>
        std::vector<AssetID> getAllIDsOfType() const {
            std::vector<AssetID> result;
            for (const auto& [id, asset] : m_AssetMap) {
                if (std::holds_alternative<T>(asset)) {
                    result.push_back(id);
                }
            }
            return result;
        }

        void remove(AssetID id) {
            m_AssetMap.erase(id);
        }

        const AssetMap& getAll() const { return m_AssetMap; }

        MaterialRaw getMaterialRaw(const Material &material);

        AssetID getCubeID() const { return m_CubeID; }
        AssetID getQuadID() const { return m_QuadID; }
        AssetID getSkyboxID() const { return m_SkyboxID; }
        AssetID getSphereID() const { return m_SphereID; }
        AssetID getDefaultMaterialID() const { return m_DefaultMatID; }

    private:
        AssetMap m_AssetMap{};
        IResourceManager* m_ResourceManager{};
        AssetID m_CubeID{};
        AssetID m_QuadID{};
        AssetID m_SkyboxID{};
        AssetID m_SphereID{};
        AssetID m_DefaultMatID{};

        // Internal generators
        MeshData createCube();
        MeshData createQuad();
        MeshData createSkybox();
        Material createDefaultMaterial(const std::string& vertPath, const std::string& fragPath,
            const std::string& geoPath);
        MeshData createSphere(float radius, unsigned int sectorCount, unsigned int stackCount);


        friend class AssetSerializer;
    };

}


