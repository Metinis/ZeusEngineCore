#pragma once
#include "ZeusEngineCore/Vertex.h"

namespace ZEN {
	struct MeshComp {
		std::vector<uint32_t> indices{};
		std::vector<Vertex> vertices{};
	};
	struct MeshDrawableComp {
		int indexCount{};
		union {
			struct {
				int vao{};
			} gl;
			struct {
				vk::Buffer buffer{};
			} vk;
		};
	};
}