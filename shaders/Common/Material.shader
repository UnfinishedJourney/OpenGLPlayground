struct Material {
    vec3 Ka;          // Ambient reflectivity
    vec3 Kd;          // Diffuse reflectivity
    vec3 Ks;          // Specular reflectivity
    float shininess;  // Specular shininess factor
};

uniform Material material;