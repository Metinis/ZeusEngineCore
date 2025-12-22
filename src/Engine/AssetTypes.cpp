#include "../../include/ZeusEngineCore/AssetTypes.h"
#include <cstring>

namespace ZEN {

#define DECL_STRING(e) #e,

    static const char* gs_TextureType[] = {
        TEXTURE_TYPE_LIST(DECL_STRING)
    };

#undef DECL_STRING

    const char* getStringTextureType(TextureType type) {
        return gs_TextureType[(int)type];
    }

    bool getEnumTextureTypeFromString(const char* str, TextureType* out) {
        for (int i = 0; i < TextureType_Count; ++i) {
            if (strcmp(gs_TextureType[i], str) == 0) {
                *out = (TextureType)i;
                return true;
            }
        }
        return false;
    }

}
