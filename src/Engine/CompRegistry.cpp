#include <dlfcn.h>
#include <ZeusEngineCore/scripting/CompRegistry.h>

using namespace ZEN;

CompRegistry* CompRegistry::s_Instance = new CompRegistry();

bool CompRegistry::loadSystemDLL(const std::string& path) {
#ifdef _WIN32
    HMODULE handle = LoadLibraryA(path.c_str());
    if (!handle) {
        std::cerr << "Failed to load DLL: " << path << "\n";
        return false;
    }
#else
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) {
        std::cerr << "Failed to load DLL: " << path << " Error: " << dlerror() << "\n";
        return false;
    }
#endif
    return true;
}