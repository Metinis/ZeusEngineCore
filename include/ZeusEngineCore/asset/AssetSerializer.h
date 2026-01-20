#pragma once
#include "AssetLibrary.h"

namespace ZEN {
    class ZEN_API AssetSerializer {
    public:
        AssetSerializer(AssetLibrary* library);
        bool serialize(const std::string& path);
        bool deserialize(const std::string& path);
    private:
        AssetLibrary* m_AssetLibrary{};
    };
}
