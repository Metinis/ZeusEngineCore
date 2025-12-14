#pragma once
#include "ZeusEngineCore/AssetLibrary.h"
#include "ZeusEngineCore/Project.h"

namespace ZEN {
    template<typename T>
    class AssetHandle {
    public:
        AssetHandle() = default;

        AssetHandle(AssetID id) : m_ID(id) {
        }

        AssetHandle(AssetID id, T&& asset) : m_ID(id) {
            Project::getActive()->getAssetLibrary()->addAsset<T>(id, std::forward<T>(asset));
        }

        AssetHandle(T&& asset) {
            m_ID = Project::getActive()->getAssetLibrary()->createAsset<T>(std::forward<T>(asset));
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
        bool valid() const { return get() != nullptr; }

    private:
        AssetID m_ID{};
    };
}
