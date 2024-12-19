#pragma once

enum class MaterialParamType {
    Ambient,    // material.Ka
    Diffuse,    // material.Kd
    Specular,   // material.Ks
    Shininess,  // material.shininess
    Roughness,  // material.roughness
    Metallic,   // material.metallic
    Emissive,   // material.emissive
};