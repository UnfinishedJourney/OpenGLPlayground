#include "Batch.h"
#include "Utilities/Logger.h"
#include "Resources/ResourceManager.h"
#include <span>
#include <numeric>
#include <stdexcept>

namespace Graphics {

    Batch::Batch(const std::string& shaderName, int materialID)
        : shaderName_(shaderName)
        , materialID_(materialID)
        , VAO_(std::make_unique<VertexArray>())
        , isDirty_(true)
    {
        // Retrieve the mesh layout (and material layout if needed) from the ResourceManager.
        auto [meshLayout, materialLayout] = ResourceManager::GetInstance().GetLayoutsFromShader(shaderName);
        meshLayout_ = meshLayout;
    }

    Batch::~Batch() = default;

    void Batch::AddRenderObject(const std::shared_ptr<BaseRenderObject>& renderObject)
    {
        if (!renderObject) {
            Logger::GetLogger()->error("Batch::AddRenderObject: received a null render object.");
            return;
        }
        renderObjects_.push_back(renderObject);
        isDirty_ = true;
    }

    const std::vector<std::shared_ptr<BaseRenderObject>>& Batch::GetRenderObjects() const
    {
        return renderObjects_;
    }

    void Batch::BuildBatches()
    {
        if (renderObjects_.empty()) {
            Logger::GetLogger()->warn("Batch::BuildBatches: no RenderObjects in batch (shader='{}', matID={}).",
                shaderName_, materialID_);
            return;
        }

        // 1) Build the VertexBufferLayout from the MeshLayout.
        VertexBufferLayout vertexLayout;
        GLuint attribIndex = 0;
        if (meshLayout_.hasPositions_) {
            vertexLayout.Push<float>(3, attribIndex++);
        }
        if (meshLayout_.hasNormals_) {
            vertexLayout.Push<float>(3, attribIndex++);
        }
        if (meshLayout_.hasTangents_) {
            vertexLayout.Push<float>(3, attribIndex++);
        }
        if (meshLayout_.hasBitangents_) {
            vertexLayout.Push<float>(3, attribIndex++);
        }
        for (size_t i = 0; i < meshLayout_.textureTypes_.size(); ++i) {
            if (meshLayout_.textureTypes_.test(i)) {
                vertexLayout.Push<float>(2, attribIndex++);
            }
        }

        // 2) Combine all vertex and index data.
        std::vector<float> combinedVertexData;
        std::vector<GLuint> combinedIndices;
        LODInfos_.clear();
        drawCommands_.clear();

        LODInfos_.reserve(renderObjects_.size());
        drawCommands_.reserve(renderObjects_.size());
        combinedVertexData.reserve(renderObjects_.size() * 500);
        combinedIndices.reserve(renderObjects_.size() * 1000);

        GLuint baseVertex = 0; // Running offset.
        for (auto& ro : renderObjects_) {
            auto mesh = ro->GetMesh();
            if (!mesh) {
                Logger::GetLogger()->error("Batch::BuildBatches: RenderObject has no valid mesh.");
                continue;
            }
            const size_t vCount = mesh->positions_.size();

            // 2a) Append vertex data.
            for (size_t i = 0; i < vCount; i++) {
                if (meshLayout_.hasPositions_ && i < mesh->positions_.size()) {
                    const auto& pos = mesh->positions_[i];
                    combinedVertexData.insert(combinedVertexData.end(), { pos.x, pos.y, pos.z });
                }
                if (meshLayout_.hasNormals_ && i < mesh->normals_.size()) {
                    const auto& nor = mesh->normals_[i];
                    combinedVertexData.insert(combinedVertexData.end(), { nor.x, nor.y, nor.z });
                }
                if (meshLayout_.hasTangents_ && i < mesh->tangents_.size()) {
                    const auto& tan = mesh->tangents_[i];
                    combinedVertexData.insert(combinedVertexData.end(), { tan.x, tan.y, tan.z });
                }
                if (meshLayout_.hasBitangents_ && i < mesh->bitangents_.size()) {
                    const auto& bitan = mesh->bitangents_[i];
                    combinedVertexData.insert(combinedVertexData.end(), { bitan.x, bitan.y, bitan.z });
                }
                for (size_t j = 0; j < meshLayout_.textureTypes_.size(); ++j) {
                    if (meshLayout_.textureTypes_.test(j)) {
                        TextureType texType = static_cast<TextureType>(j);
                        const auto uvIt = mesh->uvs_.find(texType);
                        if (uvIt != mesh->uvs_.end() && i < uvIt->second.size()) {
                            const auto& uv = uvIt->second[i];
                            combinedVertexData.insert(combinedVertexData.end(), { uv.x, uv.y });
                        }
                        else {
                            combinedVertexData.insert(combinedVertexData.end(), { 0.f, 0.f });
                        }
                    }
                }
            }

            // 2b) Build LODInfo array for this object.
            std::vector<LODInfo> lodInfos;
            lodInfos.reserve(mesh->lods_.size());
            for (const auto& lod : mesh->lods_) {
                LODInfo info;
                info.indexOffsetInCombinedBuffer = combinedIndices.size();
                info.indexCount = lod.indexCount_;
                for (size_t idx = 0; idx < lod.indexCount_; ++idx) {
                    GLuint oldIndex = mesh->indices_[lod.indexOffset_ + idx];
                    combinedIndices.push_back(oldIndex + baseVertex);
                }
                lodInfos.push_back(info);
            }
            LODInfos_.push_back(std::move(lodInfos));

            // 2c) Create an indirect draw command for this object.
            size_t lodUsed = ro->GetCurrentLOD();
            if (lodUsed >= LODInfos_.back().size())
                lodUsed = 0;
            auto& usedLOD = LODInfos_.back()[lodUsed];

            DrawElementsIndirectCommand cmd{};
            cmd.count = static_cast<GLuint>(usedLOD.indexCount);
            cmd.instanceCount = 1;
            cmd.firstIndex = static_cast<GLuint>(usedLOD.indexOffsetInCombinedBuffer);
            cmd.baseVertex = 0;
            cmd.baseInstance = 0;
            drawCommands_.push_back(cmd);

            baseVertex += static_cast<GLuint>(vCount);
        }

        // 3) Create and/or update the GPU buffers.
        {
            // Create vertex buffer.
            std::span<const std::byte> vertexSpan(
                reinterpret_cast<const std::byte*>(combinedVertexData.data()),
                combinedVertexData.size() * sizeof(float)
            );
            auto vb = std::make_unique<VertexBuffer>(vertexSpan, GL_STATIC_DRAW);

            // Create index buffer.
            std::span<const GLuint> indexSpan(
                combinedIndices.data(),
                combinedIndices.size()
            );
            auto ib = std::make_unique<IndexBuffer>(indexSpan, GL_STATIC_DRAW);

            // Create or update the indirect command buffer.
            std::span<const std::byte> cmdSpan(
                reinterpret_cast<const std::byte*>(drawCommands_.data()),
                drawCommands_.size() * sizeof(DrawElementsIndirectCommand)
            );
            if (!drawCommandBuffer_) {
                drawCommandBuffer_ = std::make_unique<IndirectBuffer>();
                drawCommandBuffer_->SetData(cmdSpan, GL_DYNAMIC_DRAW);
                if (drawCommandBuffer_->GetRendererID() == 0) {
                    Logger::GetLogger()->error("Batch::BuildBatches: failed to create a valid indirect buffer.");
                }
            }
            else {
                if (drawCommandBuffer_->GetBufferSize() != cmdSpan.size_bytes()) {
                    drawCommandBuffer_->SetData(cmdSpan, GL_DYNAMIC_DRAW);
                }
                else {
                    drawCommandBuffer_->UpdateData(cmdSpan);
                }
            }

            // Build the VAO.
            VAO_->Bind();
            VAO_->AddBuffer(*vb, vertexLayout);
            VAO_->SetIndexBuffer(*ib);
            VAO_->Unbind();
        }

        isDirty_ = false;
    }

    void Batch::Update() {
        if (isDirty_) {
            BuildBatches();
        }
    }

    void Batch::Render() const {
        if (drawCommands_.empty() || !drawCommandBuffer_)
            return; // Nothing to draw

        VAO_->Bind();
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawCommandBuffer_->GetRendererID());
        glMultiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            nullptr,
            static_cast<GLsizei>(drawCommands_.size()),
            sizeof(DrawElementsIndirectCommand)
        );
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
        VAO_->Unbind();
    }

    void Batch::CullObject(size_t objectIndex) {
        if (objectIndex >= drawCommands_.size()) {
            Logger::GetLogger()->error("Batch::CullObject: objectIndex={} out of range.", objectIndex);
            return;
        }
        auto& cmd = drawCommands_[objectIndex];
        if (cmd.count != 0) {
            cmd.count = 0;
            std::span<const std::byte> cmdSpan(
                reinterpret_cast<const std::byte*>(&cmd),
                sizeof(DrawElementsIndirectCommand)
            );
            GLintptr offset = objectIndex * sizeof(DrawElementsIndirectCommand);
            drawCommandBuffer_->UpdateData(cmdSpan, offset);
        }
    }

    void Batch::UpdateLOD(size_t objectIndex, size_t newLOD) {
        if (objectIndex >= renderObjects_.size() || objectIndex >= LODInfos_.size()) {
            Logger::GetLogger()->error("Batch::UpdateLOD: invalid objectIndex={}.", objectIndex);
            return;
        }
        auto ro = renderObjects_[objectIndex];
        if (!ro->SetLOD(newLOD))
            return; // No change

        size_t lodUsed = ro->GetCurrentLOD();
        if (lodUsed >= LODInfos_[objectIndex].size())
            lodUsed = 0;
        auto& lodRef = LODInfos_[objectIndex][lodUsed];

        auto& cmd = drawCommands_[objectIndex];
        cmd.count = static_cast<GLuint>(lodRef.indexCount);
        cmd.firstIndex = static_cast<GLuint>(lodRef.indexOffsetInCombinedBuffer);

        std::span<const std::byte> cmdSpan(
            reinterpret_cast<const std::byte*>(&cmd),
            sizeof(DrawElementsIndirectCommand)
        );
        GLintptr offset = objectIndex * sizeof(DrawElementsIndirectCommand);
        drawCommandBuffer_->UpdateData(cmdSpan, offset);
    }

} // namespace Graphics
