#pragma once
#include "ZeusEngineCore/Scene.h"
#include "ZeusEngineCore/ZENPCH.h"

namespace ZEN {
    class SceneSerializer {
    public:
        SceneSerializer(Scene* scene);
        bool serialize(const std::string& path);
        bool deserialize(const std::string& path);
    private:
        Scene* m_Scene{};
    };
}
