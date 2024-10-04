#pragma once

#include <functional>

// Macro for error checking
#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GlLogCall(#x, __FILE__, __LINE__))

// Functions for clearing and logging OpenGL errors
void GLClearError();

bool GlLogCall(const char* function, const char* file, int line);


enum class TextureType {
	Albedo,
	Normal,
	Occlusion,
	RoughnessMetallic,
	Emissive,
};

namespace std {
	template <>
	struct hash<TextureType> {
		std::size_t operator()(const TextureType& type) const {
			return std::hash<int>()(static_cast<int>(type));
		}
	};
}
