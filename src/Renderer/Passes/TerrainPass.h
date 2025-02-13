//#pragma once
//
//#include "RenderPass.h"
//#include "Graphics/Buffers/FrameBuffer.h"
//#include "Graphics/Buffers/MeshBuffer.h"
//
//class TerrainPass : public RenderPass {
//public:
//    TerrainPass(std::shared_ptr<FrameBuffer> framebuffer, const std::shared_ptr<Scene>& scene);
//    ~TerrainPass();
//
//    void Execute(const std::shared_ptr<Scene>& scene) override;
//    void UpdateFramebuffer(std::shared_ptr<FrameBuffer> framebuffer) override;
//
//private:
//    std::shared_ptr<FrameBuffer> m_Framebuffer;
//    std::shared_ptr<graphics::MeshBuffer> m_TerrainMeshBuffer;
//};