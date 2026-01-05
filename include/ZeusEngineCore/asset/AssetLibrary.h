#pragma once
#include "ZeusEngineCore/core/Util.h"
#include "../../../src/Systems/Renderer/IResourceManager.h"
#include "AssetTypes.h"

namespace ZEN {
    //cant remove these
    const std::unordered_set<std::string> defaultMeshes = {
        "Cube", "Sphere", "Capsule"
    };
    const std::unordered_set<std::string> defaultMaterials = {
        "Default"
    };
    class IResourceManager;
    class EventDispatcher;

    using AssetMap = std::unordered_map<AssetID, AssetVariant>;
    using NameMap = std::unordered_map<AssetID, std::string>;

    class AssetLibrary {
    public:
        explicit AssetLibrary();
        ~AssetLibrary();

        template<typename T>
        AssetID createAsset(T&& asset, const std::string& name = "") {
            AssetID id;
            m_ResourceManager->create(id, asset);
            m_AssetMap.emplace(id, std::forward<T>(asset));
            m_NameMap.emplace(id, name);
            return id;
        }

        template<typename T>
        void addAsset(AssetID id, T&& asset, const std::string& name = "") {
            if (!m_ResourceManager->has(id)) {
                m_ResourceManager->create(id, asset);
            }
            m_AssetMap[id] = std::forward<T>(asset);
            m_NameMap.emplace(id, name);
        }


        template <typename T>
        T* get(AssetID id) {
            auto it = m_AssetMap.find(id);
            if (it == m_AssetMap.end()) {
                //std::cout<<"Asset not found! returning nullptr: "<<id;
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
            if (m_ResourceManager->has(id)) {
                m_ResourceManager->remove(id);
            }
        }

        const AssetMap& getAll() const { return m_AssetMap; }

        std::string getName(AssetID id) {
            if(m_NameMap.contains(id)) {
                return m_NameMap[id];
            }
            return "";
        }

        MaterialRaw getMaterialRaw(const Material &material);
        MaterialRaw getMaterialRaw(const AssetID &material);

        AssetID getCubeID() const { return m_CubeID; }
        AssetID getQuadID() const { return m_QuadID; }
        AssetID getSkyboxID() const { return m_SkyboxID; }
        AssetID getSphereID() const { return m_SphereID; }
        AssetID getDefaultMaterialID() const { return m_DefaultMatID; }

    private:
        AssetMap m_AssetMap{};
        NameMap m_NameMap{};
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


