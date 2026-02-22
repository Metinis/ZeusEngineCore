#include "ZeusEngineCore/core/Project.h"

using namespace ZEN;

Project* Project::createNew() {
    s_ActiveProject = std::make_shared<Project>();
    return s_ActiveProject.get();
}

constexpr std::filesystem::path getEngineDLLPath() {
    std::filesystem::path exePath = std::filesystem::current_path();
#ifdef _WIN32
    return exePath / "ZeusEngineCore.dll";
#elif __APPLE__
    return exePath / "ZeusEngineCore.dylib";
#else
    return exePath / "libZeusEngineCore.so";
#endif
}

void Project::init(const std::string& projectRoot) {
    m_AssetLibrary = std::make_shared<AssetLibrary>();
    std::cout<<getEngineDLLPath()<<std::endl;
    std::filesystem::path root(projectRoot);
    std::filesystem::path assets = root / "assets";

    std::filesystem::create_directories(assets / "meshes");
    std::filesystem::create_directories(assets / "textures");
    std::filesystem::create_directories(assets / "scenes");
    std::filesystem::create_directories(assets / "models");
    std::filesystem::create_directories(assets / "shaders");

    std::filesystem::create_directories(assets / "scripts");
    std::filesystem::create_directories(assets / "scripts/bin");
    std::filesystem::create_directories(assets / "scripts/components");
    m_ProjectRoot = projectRoot;
}
