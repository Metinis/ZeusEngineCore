#pragma once
namespace ZEN {
	enum eRendererAPI {
		OpenGL,
		Vulkan
	};
#ifdef _WIN32
#ifdef ENGINE_EXPORTS
#define ZEN_API __declspec(dllexport)
#else
#define ZEN_API __declspec(dllimport)
#endif
#else
#define ZEN_API __attribute__((visibility("default")))
#endif
}
