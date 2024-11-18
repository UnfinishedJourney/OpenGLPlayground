#include "PostProcessingEffect.h"
#include "Utilities/Logger.h"

void PostProcessingEffect::SetupFullscreenQuad()
{
    auto& resourceManager = ResourceManager::GetInstance();

    // Retrieve the quad mesh
    auto quadMesh = resourceManager.GetMesh("quad");

    // Define the mesh layout for the quad
    MeshLayout quadMeshLayout = {
        true,  // Positions (vec2)
        false, // Normals
        false, // Tangents
        false, // Bitangents
        { TextureType::Albedo } // Texture Coordinates
    };

    // Create the mesh buffer for the quad
    m_FullscreenQuadMeshBuffer = std::make_shared<MeshBuffer>(*quadMesh, quadMeshLayout);

    if (!m_FullscreenQuadMeshBuffer) {
        Logger::GetLogger()->error("Failed to set up fullscreen quad mesh buffer.");
    }
    else {
        Logger::GetLogger()->info("Fullscreen quad mesh buffer set up successfully.");
    }
}