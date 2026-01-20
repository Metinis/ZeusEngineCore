#pragma once
#include <vector>
#include "ISystem.h"

#ifdef _WIN32
struct HINSTANCE__;
using HMODULE = HINSTANCE__*;
#endif

namespace ZEN {
    class SystemManager {
    public:
        SystemManager();
        ~SystemManager();

        bool loadSystemDLL(const std::string& path);
        bool loadAllFromDirectory(const std::string& directory, Scene* scene);

        void updateAll(float dt);
        void loadAll(Scene* scene);
        void unloadAll();
        void clearAll();

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
