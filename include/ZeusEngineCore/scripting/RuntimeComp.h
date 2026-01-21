#pragma once
#include <vector>
#include <entt/entt.hpp>

namespace ZEN {
    #ifdef _WIN32
#ifdef ENGINE_EXPORTS
#define ZEN_API __declspec(dllexport)
#else
#define ZEN_API __declspec(dllimport)
#endif
#else
#define ZEN_API __attribute__((visibility("default")))
#endif

    enum class FieldType {
        Float,
        Int,
        Bool,
    };

    struct FieldInfo {
        const char *name;
        FieldType type;
        size_t offset;
    };

    struct ComponentInfo {
        const char *name;
        size_t size;
        std::vector<FieldInfo> fields;
    };

    template<typename T>
    constexpr ZEN::FieldType DeduceFieldType();

    template<>
    constexpr ZEN::FieldType DeduceFieldType<int>() { return ZEN::FieldType::Int; }

    template<>
    constexpr ZEN::FieldType DeduceFieldType<float>() { return ZEN::FieldType::Float; }

    template<>
    constexpr ZEN::FieldType DeduceFieldType<bool>() { return ZEN::FieldType::Bool; }


#define FIELD(Type, Member)                                  \
ZEN::FieldInfo {                                         \
#Member,                                             \
ZEN::DeduceFieldType<decltype(Type::Member)>(),      \
offsetof(Type, Member)                               \
}
#define REGISTER_COMPONENT(Type, ...)                        \
namespace {                                              \
struct Type##_Registrar {                            \
Type##_Registrar() {                             \
ZEN::Application::get().getEngine()->getCompRegistry().registerComponent(    \
ZEN::ComponentInfo{                      \
#Type,                               \
sizeof(Type),                        \
{ __VA_ARGS__ }                      \
}                                        \
);                                           \
}                                                \
};                                                   \
inline Type##_Registrar s_##Type##_registrar;        \
}

    struct RuntimeComponentStorage {
        std::unordered_map<entt::entity, std::vector<uint8_t>> data;
    };

    struct ZEN_API RuntimeComponent {
        std::vector<uint8_t> buffer;
        const ComponentInfo *info;

        void* getFieldPtr(const std::string_view name) {
            for (const auto &field: info->fields) {
                if (std::strcmp(field.name, name.data()) == 0) {
                    return buffer.data() + field.offset;
                }
            }
            return nullptr;
        }
        template<typename T>
        T &getField(const char *fieldName) {
            void *ptr = getFieldPtr(fieldName);
            ENTT_ASSERT(ptr, "Field not found");
            return *reinterpret_cast<T *>(ptr);
        }
    };


#define ZEN_GET_ENTITIES(COMP) \
    m_Scene->getEntities(#COMP)


#define ZEN_GET_FIELD(COMP, ENTITY, FIELD) \
ENTITY.getRuntimeField<decltype(COMP::FIELD)>(#COMP, #FIELD)

#define ZEN_GET_FIELD_S(COMP, ENTITY, FIELD) \
ENTITY.getRuntimeField<decltype(COMP::FIELD)>(COMP, #FIELD)

#define ZEN_SET_FIELD(COMP, ENTITY, FIELD, VALUE) \
(ZEN_GET(COMP, ENTITY, FIELD) = (VALUE))
}

