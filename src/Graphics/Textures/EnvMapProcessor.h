#pragma once

#include <glm/glm.hpp>
#include <string>
#include <filesystem>

#include "Bitmap.h"

class EnvMapPreprocessor
{
public:
	void SaveAsVerticalCross(const std::filesystem::path& texturePath, const std::filesystem::path& outPath) const;

private:
	Bitmap LoadEquirectangularTexture(const std::filesystem::path& texturePath) const;
	Bitmap ConvertEquirectangularMapToVerticalCross(const Bitmap& b) const;
	Bitmap ConvertVerticalCrossToCubeMapFaces(const Bitmap& b) const;

	inline Bitmap ConvertEquirectangularMapToCubeMapFaces(const Bitmap& b) const {
		return ConvertVerticalCrossToCubeMapFaces(ConvertEquirectangularMapToVerticalCross(b));
	}

	void ConvolveDiffuse(const glm::vec3* data, int srcW, int srcH, int dstW, int dstH, glm::vec3* output, int numMonteCarloSamples) const;

	void SaveAsHDR(const Bitmap& image, const std::filesystem::path& outPath) const;
};