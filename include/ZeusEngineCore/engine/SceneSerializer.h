#pragma once
#include "ZeusEngineCore/engine/Scene.h"
#include "ZeusEngineCore/ZENPCH.h"

namespace ZEN {
    class ZEN_API SceneSerializer {
    public:
        SceneSerializer(Scene* scene);
        bool serialize(const std::string& path);
        bool deserialize(const std::string& path);
    private:
        Scene* m_Scene{};
    };
}
