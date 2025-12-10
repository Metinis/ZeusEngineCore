#pragma once
#include "AssetLibrary.h"

namespace ZEN {
    class AssetSerializer {
    public:
        AssetSerializer(AssetLibrary* library);
        bool serialize(const std::string& path);
        bool deserialize(const std::string& path);
    private:
        AssetLibrary* m_AssetLibrary{};
    };
}
