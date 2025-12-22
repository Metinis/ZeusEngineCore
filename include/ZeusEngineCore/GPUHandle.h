#pragma once
#include "Application.h"
#include "../../src/Systems/Renderer/IResourceManager.h"

namespace ZEN {
    template<typename T>
    class GPUHandle {
    public:
        GPUHandle() = default;

        //todo maybe make resource manager static
        GPUHandle(AssetID id) : m_ID(id) {

        }

        GPUHandle(AssetID id, T&& gpuRes, const std::string& name = "") :
            m_ID(id) {
            //todo create here
        }

        GPUHandle(T&& gpuRes, const std::string& name = "") {
            //todo create here
        }

        T *get() const {
            if (m_ID != 0) {
                return Application::get().getEngine()->getRenderer().getResourceManager()->get<T>(m_ID);
            }
            return nullptr;
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
