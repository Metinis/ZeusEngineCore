#include "ZeusEngineCore/Project.h"

using namespace ZEN;

Project* Project::createNew() {
    s_ActiveProject = std::make_shared<Project>();
    return s_ActiveProject.get();
}

void Project::init() {
    m_AssetLibrary = std::make_shared<AssetLibrary>();
}
