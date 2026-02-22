#include "ZeusEngineCore/scripting/SystemManager.h"
#include <vector>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

using namespace ZEN;

SystemManager::SystemManager() = default;

SystemManager::~SystemManager() {
    clearAll();
}
#if defined(_WIN32)
constexpr const char* DYLIB_EXT = ".dll";
#elif defined(__APPLE__)
constexpr const char* DYLIB_EXT = ".dylib";
#else
constexpr const char* DYLIB_EXT = ".so";
#endif



bool SystemManager::loadSystemDLL(const std::string& path) {
#ifdef _WIN32
    HMODULE handle = LoadLibraryA(path.c_str());
    if (!handle) {
        std::cerr << "Failed to load DLL: " << path << "\n";
        return false;
    }
    m_DllHandles.push_back(handle);
    auto createFunc = reinterpret_cast<ISystem*(*)()>(GetProcAddress(handle, "createScriptSystem"));
#else
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) {
        std::cerr << "Failed to load DLL: " << path << " Error: " << dlerror() << "\n";
        return false;
    }
    m_DllHandles.push_back(handle);
    auto createFunc = reinterpret_cast<ISystem*(*)()>(dlsym(handle, "createScriptSystem"));
#endif

    if (!createFunc) {
        std::cerr << "Failed to find createScriptSystem in: " << path << "\n";
        return false;
    }

    ISystem* system = createFunc();
    if (!system) {
        std::cerr << "Failed to create system from DLL: " << path << "\n";
        return false;
    }

    m_Systems.push_back(system);
    return true;
}
bool SystemManager::loadAllFromDirectory(const std::string& directory, Scene* scene) {
    if (!std::filesystem::exists(directory)) {
        std::cerr << "Directory does not exist: " << directory << "\n";
        return false;
    }

    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (!entry.is_regular_file())
            continue;

        const auto& path = entry.path();
        if (path.extension() != DYLIB_EXT)
            continue;

        std::cout << "Loading system: " << path << "\n";

        if (!loadSystemDLL(path.string())) {
            std::cerr << "Failed to load: " << path << "\n";
        }
    }

    return true;
}
void SystemManager::updateAll(float dt) {
    for (auto* s : m_Systems)
        s->onUpdate(dt);
}

void SystemManager::collisionAll(const CollisionEvent& e) {
    for (auto* s : m_Systems)
        s->onCollision(e);
}

void SystemManager::loadAll(Scene* scene) {
    for (auto* s : m_Systems) {
        s->onLoad(scene);
    }
}

void SystemManager::unloadAll() {
    for (auto* s : m_Systems) {
        s->onUnload();
    }
}

void SystemManager::clearAll() {
    for (auto* s : m_Systems) {
        s->onUnload();
        delete s;
    }
    m_Systems.clear();
    for (auto h : m_DllHandles) {
#ifdef _WIN32
        for (auto h : m_DllHandles)
            FreeLibrary(h);
#else
        for (auto h : m_DllHandles)
            dlclose(h);
#endif
    }
    m_DllHandles.clear();
}

void SystemManager::addSystem(ISystem* system) {
    m_Systems.push_back(system);
}
