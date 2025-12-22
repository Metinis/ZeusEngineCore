#pragma once
#include <ZeusEngineCore/AssetLibrary.h>

namespace ZEN {
    class Project {
    public:
        static Project* getActive() { return s_ActiveProject.get(); }
        static Project* createNew();
        void init(IResourceManager* resourceManager);
        std::shared_ptr<AssetLibrary> getAssetLibrary() { return m_AssetLibrary; }
    private:
        std::shared_ptr<AssetLibrary> m_AssetLibrary;
        inline static std::shared_ptr<Project> s_ActiveProject;
    };
}
