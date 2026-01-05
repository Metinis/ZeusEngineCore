#pragma once
#include "AssetLibrary.h"
#include "ZeusEngineCore/core/Project.h"

namespace ZEN {
    template<typename T>
    class AssetHandle {
    public:
        AssetHandle() = default;

        AssetHandle(AssetID id) : m_ID(id) {
        }

        AssetHandle(AssetID id, T&& asset, const std::string& name = "") : m_ID(id) {
            Project::getActive()->getAssetLibrary()->addAsset<T>(id, std::forward<T>(asset), name);
        }

        AssetHandle(T&& asset, const std::string& name = "") {
            m_ID = Project::getActive()->getAssetLibrary()->createAsset<T>(std::forward<T>(asset), name);
        }

        T *get() const {
            return Project::getActive()->getAssetLibrary()->get<T>(m_ID);
        }

        T &operator*() const {
            return *get();
        }

        T *operator->() const {
            return get();
        }

        AssetID id() const { return m_ID; }
        void setID(AssetID id) {m_ID = id;}
        bool valid() const { return get() != nullptr; }

    private:
        AssetID m_ID{};
    };
}
