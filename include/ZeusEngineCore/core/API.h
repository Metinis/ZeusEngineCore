#pragma once
struct ImGuiContext;
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
#pragma once
#ifdef _WIN32
#ifdef PLUGIN_EXPORTS
#define PLUGIN_API __declspec(dllexport) 
#else
#define PLUGIN_API __declspec(dllimport) 
#endif
#else
#define PLUGIN_API __attribute__((visibility("default"))) 
#endif

	
	extern "C" ZEN_API ImGuiContext* getEngineImGuiContext();
}

