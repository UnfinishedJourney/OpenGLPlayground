#pragma once
#include <string>
#include <unordered_set>

// This enum can represent different advanced parameters or textures.
enum class MaterialParamType {
    Ambient,
    Diffuse,
    Specular,
    Shininess,
    Roughness,
    Metallic,
    Emissive,
    Custom  // For additional user-defined parameters
};

// Similar to MeshLayout, we define boolean flags for each parameter.
// You might also define sets for texture usage if needed.
struct MaterialLayout {
    bool hasAmbient = false;
    bool hasDiffuse = false;
    bool hasSpecular = false;
    bool hasShininess = false;
    bool hasRoughness = false;
    bool hasMetallic = false;
    bool hasEmissive = false;
    bool hasCustom = false; // Some custom parameters

    bool operator==(const MaterialLayout& other) const {
        return hasAmbient == other.hasAmbient &&
            hasDiffuse == other.hasDiffuse &&
            hasSpecular == other.hasSpecular &&
            hasShininess == other.hasShininess &&
            hasRoughness == other.hasRoughness &&
            hasMetallic == other.hasMetallic &&
            hasEmissive == other.hasEmissive &&
            hasCustom == other.hasCustom;
    }
};