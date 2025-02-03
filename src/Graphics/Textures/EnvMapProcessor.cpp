#include <cstdio>
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "EnvMapProcessor.h"
#include <stb_image_resize2.h>
#include <stb_image_write.h>
#include <stb_image.h>
#include <stdexcept>
#include "Utilities/Logger.h"

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::ivec2;

namespace Math
{
	static constexpr float PI = 3.14159265359f;
	static constexpr float TWOPI = 6.28318530718f;
}

template <typename T>
T clamp(T v, T a, T b)
{
	if (v < a) return a;
	if (v > b) return b;
	return v;
}

/// From Henry J. Warren's "Hacker's Delight"
float radicalInverse_VdC(uint32_t bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10f; // / 0x100000000
}

// The i-th point is then computed by

/// From http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html

vec2 hammersley2d(uint32_t i, uint32_t N)
{
	return vec2(float(i) / float(N), radicalInverse_VdC(i));
}

vec3 faceCoordsToXYZ(int i, int j, int faceID, int faceSize)
{
	const float A = 2.0f * float(i) / faceSize;
	const float B = 2.0f * float(j) / faceSize;

	if (faceID == 0) return vec3(-1.0f, A - 1.0f, B - 1.0f);
	if (faceID == 1) return vec3(A - 1.0f, -1.0f, 1.0f - B);
	if (faceID == 2) return vec3(1.0f, A - 1.0f, 1.0f - B);
	if (faceID == 3) return vec3(1.0f - A, 1.0f, 1.0f - B);
	if (faceID == 4) return vec3(B - 1.0f, A - 1.0f, 1.0f);
	if (faceID == 5) return vec3(1.0f - B, A - 1.0f, -1.0f);

	return vec3();
}

glm::vec3 ImportanceSampleGGX(const glm::vec2 Xi, const glm::vec3& N, float roughness)
{
	// Convert roughness to alpha (here we use a simple square: alpha = roughness^2)
	float a = roughness * roughness;

	// Sample spherical coordinates with the GGX distribution.
	float phi = 2.0f * Math::PI * Xi.x;
	// Compute cosine of theta using the inverse CDF of the GGX distribution.
	float cosTheta = sqrt((1.0f - Xi.y) / (1.0f + (a * a - 1.0f) * Xi.y));
	float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

	// In tangent space, H points along:
	glm::vec3 H;
	H.x = sinTheta * cos(phi);
	H.y = sinTheta * sin(phi);
	H.z = cosTheta;

	// Transform H from tangent space to world space.
	// Build an orthonormal basis (N, tangentX, tangentY) around N.
	glm::vec3 up = fabs(N.z) < 0.999f ? glm::vec3(0.0f, 0.0f, 1.0f)
		: glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 tangentX = glm::normalize(glm::cross(up, N));
	glm::vec3 tangentY = glm::cross(N, tangentX);
	return glm::normalize(tangentX * H.x + tangentY * H.y + N * H.z);
}

// Sample the equirectangular environment map.
// This function converts a 3D direction into (u,v) coordinates and fetches a bilinearly
// interpolated color from the input Bitmap.
glm::vec3 SampleEquirectangular(const Bitmap& envMap, const glm::vec3& dir)
{
	// Convert the 3D direction to spherical coordinates.
	// (Note: adjust the formulas if your equirectangular mapping is different.)
	float u = 0.5f + atan2(dir.z, dir.x) / (2.0f * Math::PI);
	float v = acos(glm::clamp(dir.y, -1.0f, 1.0f)) / Math::PI;

	// Convert to pixel coordinates.
	float xf = u * envMap.w_;
	float yf = v * envMap.h_;
	int clampW = envMap.w_ - 1;
	int clampH = envMap.h_ - 1;
	int x0 = clamp((int)floor(xf), 0, clampW);
	int y0 = clamp((int)floor(yf), 0, clampH);
	int x1 = clamp(x0 + 1, 0, clampW);
	int y1 = clamp(y0 + 1, 0, clampH);

	float s = xf - x0;
	float t = yf - y0;

	glm::vec4 A = envMap.getPixel(x0, y0);
	glm::vec4 B = envMap.getPixel(x1, y0);
	glm::vec4 C = envMap.getPixel(x0, y1);
	glm::vec4 D = envMap.getPixel(x1, y1);

	glm::vec4 color = A * (1.0f - s) * (1.0f - t) +
		B * s * (1.0f - t) +
		C * (1.0f - s) * t +
		D * s * t;
	return glm::vec3(color);
}

void EnvMapPreprocessor::ConvolveDiffuse(const vec3* data, int srcW, int srcH, int dstW, int dstH, vec3* output, int numMonteCarloSamples) const
{
	// only equirectangular maps are supported
	assert(srcW == 2 * srcH);

	if (srcW != 2 * srcH) return;

	std::vector<vec3> tmp(dstW * dstH);

	stbir_resize(reinterpret_cast<const float*>(data),
		srcW,
		srcH,
		0,
		reinterpret_cast<float*>(tmp.data()),
		dstW,
		dstH,
		0,
		STBIR_RGB,
		STBIR_TYPE_FLOAT,
		STBIR_EDGE_CLAMP,
		STBIR_FILTER_CUBICBSPLINE);

	const vec3* scratch = tmp.data();
	srcW = dstW;
	srcH = dstH;

	for (int y = 0; y != dstH; y++)
	{
		printf("Line %i...\n", y);
		const float theta1 = float(y) / float(dstH) * Math::PI;
		for (int x = 0; x != dstW; x++)
		{
			const float phi1 = float(x) / float(dstW) * Math::TWOPI;
			const vec3 V1 = vec3(sin(theta1) * cos(phi1), sin(theta1) * sin(phi1), cos(theta1));
			vec3 color = vec3(0.0f);
			float weight = 0.0f;
			for (int i = 0; i != numMonteCarloSamples; i++)
			{
				const vec2 h = hammersley2d(i, numMonteCarloSamples);
				const int x1 = int(floor(h.x * srcW));
				const int y1 = int(floor(h.y * srcH));
				const float theta2 = float(y1) / float(srcH) * Math::PI;
				const float phi2 = float(x1) / float(srcW) * Math::TWOPI;
				const vec3 V2 = vec3(sin(theta2) * cos(phi2), sin(theta2) * sin(phi2), cos(theta2));
				const float D = std::max(0.0f, glm::dot(V1, V2));
				if (D > 0.01f)
				{
					color += scratch[y1 * srcW + x1] * D;
					weight += D;
				}
			}
			output[y * dstW + x] = color / weight;
		}
	}
}

void EnvMapPreprocessor::SaveAsVerticalCross(const std::filesystem::path& texturePath, const std::filesystem::path& outPath) const
{
	Bitmap bm = LoadTexture(texturePath);
	Bitmap vcbm = ConvertEquirectangularMapToVerticalCross(bm);
	SaveAsHDR(vcbm, outPath);
}

Bitmap EnvMapPreprocessor::LoadTexture(const std::filesystem::path& texturePath) const
{
	int w, h, comp;
	const float* img = stbi_loadf(texturePath.string().c_str(), &w, &h, &comp, 3);
	Bitmap tex(w, h, comp, eBitmapFormat_Float, img);

	return tex;
}

Bitmap EnvMapPreprocessor::ConvertEquirectangularMapToVerticalCross(const Bitmap& b) const
{
	if (b.type_ != eBitmapType_2D) return Bitmap();

	const int faceSize = b.w_ / 4;

	const int w = faceSize * 3;
	const int h = faceSize * 4;

	Bitmap result(w, h, b.comp_, b.fmt_);

	const ivec2 kFaceOffsets[] =
	{
		ivec2(faceSize, faceSize * 3),
		ivec2(0, faceSize),
		ivec2(faceSize, faceSize),
		ivec2(faceSize * 2, faceSize),
		ivec2(faceSize, 0),
		ivec2(faceSize, faceSize * 2)
	};

	const int clampW = b.w_ - 1;
	const int clampH = b.h_ - 1;

	for (int face = 0; face != 6; face++)
	{
		for (int i = 0; i != faceSize; i++)
		{
			for (int j = 0; j != faceSize; j++)
			{
				const vec3 P = faceCoordsToXYZ(i, j, face, faceSize);
				const float R = hypot(P.x, P.y);
				const float theta = atan2(P.y, P.x);
				const float phi = atan2(P.z, R);
				//	float point source coordinates
				const float Uf = float(2.0f * faceSize * (theta + Math::PI) / Math::PI);
				const float Vf = float(2.0f * faceSize * (Math::PI / 2.0f - phi) / Math::PI);
				// 4-samples for bilinear interpolation
				const int U1 = clamp(int(floor(Uf)), 0, clampW);
				const int V1 = clamp(int(floor(Vf)), 0, clampH);
				const int U2 = clamp(U1 + 1, 0, clampW);
				const int V2 = clamp(V1 + 1, 0, clampH);
				// fractional part
				const float s = Uf - U1;
				const float t = Vf - V1;
				// fetch 4-samples
				const vec4 A = b.getPixel(U1, V1);
				const vec4 B = b.getPixel(U2, V1);
				const vec4 C = b.getPixel(U1, V2);
				const vec4 D = b.getPixel(U2, V2);
				// bilinear interpolation
				const vec4 color = A * (1 - s) * (1 - t) + B * (s) * (1 - t) + C * (1 - s) * t + D * (s) * (t);
				result.setPixel(i + kFaceOffsets[face].x, j + kFaceOffsets[face].y, color);
			}
		};
	}

	return result;
}

Bitmap EnvMapPreprocessor::ConvertVerticalCrossToCubeMapFaces(const Bitmap& b) const
{
	const int faceWidth = b.w_ / 3;
	const int faceHeight = b.h_ / 4;

	Bitmap cubemap(faceWidth, faceHeight, 6, b.comp_, b.fmt_);
	cubemap.type_ = eBitmapType_Cube;

	const uint8_t* src = b.data_.data();
	uint8_t* dst = cubemap.data_.data();

	/*
			------
			| +Y |
	 ----------------
	 | -X | -Z | +X |
	 ----------------
			| -Y |
			------
			| +Z |
			------
	*/

	const int pixelSize = cubemap.comp_ * Bitmap::getBytesPerComponent(cubemap.fmt_);

	for (int face = 0; face != 6; ++face)
	{
		for (int j = 0; j != faceHeight; ++j)
		{
			for (int i = 0; i != faceWidth; ++i)
			{
				int x = 0;
				int y = 0;

				switch (face)
				{
					// GL_TEXTURE_CUBE_MAP_POSITIVE_X
				case 0:
					x = i;
					y = faceHeight + j;
					break;

					// GL_TEXTURE_CUBE_MAP_NEGATIVE_X
				case 1:
					x = 2 * faceWidth + i;
					y = 1 * faceHeight + j;
					break;

					// GL_TEXTURE_CUBE_MAP_POSITIVE_Y
				case 2:
					x = 2 * faceWidth - (i + 1);
					y = 1 * faceHeight - (j + 1);
					break;

					// GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
				case 3:
					x = 2 * faceWidth - (i + 1);
					y = 3 * faceHeight - (j + 1);
					break;

					// GL_TEXTURE_CUBE_MAP_POSITIVE_Z
				case 4:
					x = 2 * faceWidth - (i + 1);
					y = b.h_ - (j + 1);
					break;

					// GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
				case 5:
					x = faceWidth + i;
					y = faceHeight + j;
					break;
				}

				memcpy(dst, src + (y * b.w_ + x) * pixelSize, pixelSize);

				dst += pixelSize;
			}
		}
	}

	return cubemap;
}

Bitmap EnvMapPreprocessor::ComputeIrradianceEquirect(const Bitmap& inEquirect,
	int outW,
	int outH,
	int samples) const
{
	// Must be float, 3 or 4 components, w=2*h, etc.
	if (inEquirect.fmt_ != eBitmapFormat_Float) {
		throw std::runtime_error("ComputeIrradianceEquirect: input must be float format!");
	}
	if (inEquirect.data_.empty()) {
		throw std::runtime_error("ComputeIrradianceEquirect: input is empty!");
	}

	int srcW = inEquirect.w_;
	int srcH = inEquirect.h_;
	if (srcW != 2 * srcH) {
		throw std::runtime_error("ComputeIrradianceEquirect: equirect input should have width=2*height!");
	}

	// We’ll store the result in a float, 3-component eBitmapType_2D
	Bitmap outBmp(outW, outH, 3, eBitmapFormat_Float);

	// We re-use your `ConvolveDiffuse(...)`, which expects raw pointers:
	//    ConvolveDiffuse(const vec3* data, int srcW, int srcH, int dstW, int dstH,
	//                    vec3* output, int numMonteCarloSamples)
	// So let's just cast the data pointers.

	// Convert input data_ to `vec3*`
	// (If comp==3, it’s direct, if comp==4, we skip alpha. For simplicity, assume comp=3.)
	const vec3* srcData = reinterpret_cast<const vec3*>(inEquirect.data_.data());
	// The output is outBmp.data_.data() => also float, 3 channels
	vec3* dstData = reinterpret_cast<vec3*>(outBmp.data_.data());

	ConvolveDiffuse(srcData, srcW, srcH, outW, outH, dstData, samples);

	// Mark it eBitmapType_2D
	outBmp.type_ = eBitmapType_2D;
	return outBmp;
}

void EnvMapPreprocessor::SaveAsHDR(const Bitmap& image, const std::filesystem::path& outPath) const
{
	// Must be float
	if (image.fmt_ != eBitmapFormat_Float) {
		throw std::runtime_error("SaveAsHDR: image must be float format!");
	}

	// stbi_write_hdr can handle comp=3 or comp=4
	int w = image.w_;
	int h = image.h_;
	int c = image.comp_;
	if (c < 3 || c > 4) {
		throw std::runtime_error("SaveAsHDR: only 3 or 4 channels supported!");
	}
	if (image.d_ != 1) {
		// stbi can't handle 3D/cubemap directly; you'd flatten or save faces.
		throw std::runtime_error("SaveAsHDR: cannot save 3D/cubemap directly, flatten first!");
	}

	const float* ptr = reinterpret_cast<const float*>(image.data_.data());
	if (!stbi_write_hdr(outPath.string().c_str(), w, h, c, ptr)) {
		throw std::runtime_error("stbi_write_hdr failed!");
	}
}


std::vector<Bitmap> EnvMapPreprocessor::ComputePrefilteredCubemap(const Bitmap& inEquirect, int baseFaceSize, int numSamples) const
{
	// Determine number of mip levels (including level 0).
	int mipLevels = (int)floor(log2((float)baseFaceSize)) + 1;
	std::vector<Bitmap> mipMaps;
	mipMaps.reserve(mipLevels);

	// For each mip level...
	for (int mip = 0; mip < mipLevels; mip++)
	{
		int faceSize = baseFaceSize >> mip; // faceSize = baseFaceSize / 2^mip
		// Allocate a cubemap Bitmap for this mip level.
		// (Assumes Bitmap(w, h, numFaces, comp, fmt) constructs a cube-map image.)
		Bitmap mipCubemap(faceSize, faceSize, 6, inEquirect.comp_, inEquirect.fmt_);
		mipCubemap.type_ = eBitmapType_Cube;

		// Roughness increases with mip level.
		float roughness = (mipLevels > 1) ? float(mip) / float(mipLevels - 1) : 0.0f;

		// For each face of the cubemap...
		for (int face = 0; face < 6; face++)
		{
			// Loop over every texel in the face.
			for (int y = 0; y < faceSize; y++)
			{
				for (int x = 0; x < faceSize; x++)
				{
					// Use pixel center coordinates:
					float u = (2.0f * (x + 0.5f) / faceSize) - 1.0f;
					float v = (2.0f * (y + 0.5f) / faceSize) - 1.0f;
					// Map (u,v) to a 3D direction vector according to the face.
					glm::vec3 R;
					if (face == 0)       // +X
						R = glm::vec3(1.0f, -v, -u);
					else if (face == 1)  // -X
						R = glm::vec3(-1.0f, -v, u);
					else if (face == 2)  // +Y
						R = glm::vec3(u, 1.0f, v);
					else if (face == 3)  // -Y
						R = glm::vec3(u, -1.0f, -v);
					else if (face == 4)  // +Z
						R = glm::vec3(u, -v, 1.0f);
					else if (face == 5)  // -Z
						R = glm::vec3(-u, -v, -1.0f);
					R = glm::normalize(R);

					glm::vec3 prefilteredColor(0.0f);
					float totalWeight = 0.0f;
					// Monte Carlo integration over the hemisphere.
					for (int i = 0; i < numSamples; i++)
					{
						glm::vec2 Xi = hammersley2d(i, numSamples);
						// Importance-sample a half vector using GGX.
						glm::vec3 H = ImportanceSampleGGX(Xi, R, roughness);
						// Compute the reflection direction using the sampled half vector.
						glm::vec3 L = glm::normalize(2.0f * glm::dot(R, H) * H - R);
						float NdotL = glm::max(glm::dot(R, L), 0.0f);
						if (NdotL > 0.0f)
						{
							glm::vec3 sampleColor = SampleEquirectangular(inEquirect, L);
							prefilteredColor += sampleColor * NdotL;
							totalWeight += NdotL;
						}
					}
					prefilteredColor /= totalWeight;
					mipCubemap.setPixel(x, y, face, glm::vec4(prefilteredColor, 1.0f));
				}
			}
		}
		mipMaps.push_back(mipCubemap);
	}
	return mipMaps;
}

void EnvMapPreprocessor::SaveAsLDR(const Bitmap& image, const std::filesystem::path& outPath) const {
	// Assume 'image' uses floating point data in [0, ...]. We map each pixel into [0,1],
	// apply gamma correction (simple tone mapping), then scale to [0,255] for PNG.
	int w = image.w_;
	int h = image.h_;
	int comp = image.comp_;
	std::vector<uint8_t> ldrData(w * h * comp);

	// Pointer to the HDR float data.
	const float* src = reinterpret_cast<const float*>(image.data_.data());
	for (int i = 0; i < w * h * comp; i++) {
		// Clamp to [0,1]. (You could apply a more sophisticated tone mapping here.)
		float value = src[i];
		value = glm::clamp(value, 0.0f, 1.0f);
		// Apply a gamma curve (simple approximation, gamma=2.2)
		value = pow(value, 1.0f / 2.2f);
		ldrData[i] = static_cast<uint8_t>(value * 255.0f);
	}

	// Write PNG using stb_image_write.
	if (!stbi_write_png(outPath.string().c_str(), w, h, comp, ldrData.data(), w * comp)) {
		throw std::runtime_error("stbi_write_png failed in SaveAsLDR!");
	}
}

// NEW: SaveFacesToDiskLDR – save 6 faces of a cube-map (contained in a Bitmap) as LDR images.
void EnvMapPreprocessor::SaveFacesToDiskLDR(const Bitmap& cubeMap, const std::array<std::filesystem::path, 6>& facePaths, const std::string& prefix) const {
	if (cubeMap.d_ != 6) {
		Logger::GetLogger()->error("SaveFacesToDiskLDR: expected 6 faces (d_ == 6), got d_ = {}", cubeMap.d_);
		return;
	}

	int faceW = cubeMap.w_;
	int faceH = cubeMap.h_;
	int comp = cubeMap.comp_;
	int pixelSize = comp * Bitmap::getBytesPerComponent(cubeMap.fmt_);
	int faceSizeBytes = faceW * faceH * pixelSize;

	// For each face, extract its data and save as LDR.
	for (int face = 0; face < 6; face++) {
		Bitmap faceBmp(faceW, faceH, comp, cubeMap.fmt_);
		const uint8_t* src = cubeMap.data_.data() + face * faceSizeBytes;
		std::memcpy(faceBmp.data_.data(), src, faceSizeBytes);

		try {
			SaveAsLDR(faceBmp, facePaths[face]);
		}
		catch (const std::exception& e) {
			Logger::GetLogger()->error("SaveFacesToDiskLDR: face {}: {}", prefix + std::to_string(face), e.what());
		}
	}
}