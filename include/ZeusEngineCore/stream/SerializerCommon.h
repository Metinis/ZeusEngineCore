#pragma once
#include <yaml-cpp/yaml.h>

#include "ZeusEngineCore/asset/AssetTypes.h"

/* Add operators for glm */
namespace YAML {
    template<>
    struct convert<glm::vec3> {
        static Node encode(const glm::vec3& rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec3& rhs) {
            if (!node.IsSequence() || node.size() != 3)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };
    template<>
    struct convert<glm::vec2> {
        static Node encode(const glm::vec2& rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec2& rhs) {
            if (!node.IsSequence() || node.size() != 2)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            return true;
        }
    };
    template<>
    struct convert<ZEN::TextureType> {
        static Node encode(const ZEN::TextureType& rhs) {
            Node node;
            node.push_back(rhs);
            node.SetStyle(EmitterStyle::Default);
            return node;
        }

        static bool decode(const Node& node, ZEN::TextureType& rhs) {
            if (!node.IsScalar())
                return false;

            const std::string value = node.as<std::string>();
            if (!ZEN::getEnumTextureTypeFromString(value.c_str(), &rhs)) {
                throw YAML::BadConversion(node.Mark());
            }
            return true;
        }
    };
    inline Emitter &operator<<(Emitter &emitter, const glm::vec2 &v) {
        emitter << Flow;
        emitter << BeginSeq << v.x << v.y << EndSeq;
        return emitter;
    }
    inline Emitter &operator<<(Emitter &emitter, const glm::vec3 &v) {
        emitter << Flow;
        emitter << BeginSeq << v.x << v.y << v.z << EndSeq;
        return emitter;
    }
    inline Emitter &operator<<(Emitter &emitter, const glm::vec4 &v) {
        emitter << Flow;
        emitter << BeginSeq << v.x << v.y << v.z << v.w << EndSeq;
        return emitter;
    }
    inline Emitter &operator<<(Emitter &emitter, const ZEN::TextureType &v) {
        emitter << ZEN::getStringTextureType(v);
        return emitter;
    }
}