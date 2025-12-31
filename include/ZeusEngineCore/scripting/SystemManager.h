#pragma once
#include <vector>
#include "ISystem.h"

namespace ZEN {
    class SystemManager {
    public:
        SystemManager();
        ~SystemManager();

        bool loadSystemDLL(const std::string& path, Scene* scene);
        bool loadAllFromDirectory(const std::string& directory, Scene* scene);

        void updateAll(float dt);
        void loadAll(Scene* scene);
        void unloadAll();

        void addSystem(ISystem* system);
    private:
        std::vector<ISystem*> m_Systems;
#ifdef _WIN32
        std::vector<HMODULE> m_DllHandles;
#else
        std::vector<void*> m_DllHandles;
#endif
    };
}
