#include "Batch.h"
#include "Utilities/Logger.h"
#include "Resources/ResourceManager.h"
#include <span>
#include <numeric>
#include <stdexcept>
#include <algorithm> // for std::max

namespace Graphics {

    // ========================= Batch Public Methods =========================

    Batch::Batch(const std::string& shaderName, int materialID)
        : shaderName_(shaderName),
        materialID_(materialID),
        isDirty_(true)
    {
        // Retrieve the mesh layout (and material layout if needed) from ResourceManager.
        auto [meshLayout, materialLayout] = ResourceManager::GetInstance().GetLayoutsFromShader(shaderName);
        meshLayout_ = meshLayout;
    }

    Batch::~Batch() = default;

    void Batch::AddRenderObject(const std::shared_ptr<BaseRenderObject>& renderObject) {
        if (!renderObject) {
            Logger::GetLogger()->error("Batch::AddRenderObject: received a null render object.");
            return;
        }
        if (renderObject->GetMaterialID() != materialID_ || renderObject->GetMeshLayout() != meshLayout_) {
            Logger::GetLogger()->error("Batch::AddRenderObject: object and batch don't match.");
            return;
        }
        renderObjects_.push_back(renderObject);
        isDirty_ = true;
    }

    const std::vector<std::shared_ptr<BaseRenderObject>>& Batch::GetRenderObjects() const {
        return renderObjects_;
    }

    void Batch::BuildBatches() {
        if (renderObjects_.empty()) {
            Logger::GetLogger()->warn("Batch::BuildBatches: no RenderObjects in batch (shader='{}', matID={}).",
                shaderName_, materialID_);
            return;
        }

        // 1) Build the vertex layout and compute totals.
        VertexBufferLayout vertexLayout;
        BatchGeometryTotals totals = BuildLayoutAndTotals(vertexLayout);

        // Reserve combined arrays.
        std::vector<float> combinedVertexData;
        combinedVertexData.reserve(totals.totalVertices * totals.vertexElementCount);
        std::vector<GLuint> combinedIndices;
        combinedIndices.reserve(totals.totalIndices);
        // Clear and reserve lod infos and draw commands.
        lodInfos_.clear();
        drawCommands_.clear();
        lodInfos_.reserve(renderObjects_.size());
        drawCommands_.reserve(renderObjects_.size());

        // 2) Combine geometry data (vertex attributes, indices, LOD info, draw commands).
        GLuint baseVertex = 0;
        CombineGeometryData(combinedVertexData, combinedIndices, lodInfos_, drawCommands_, baseVertex);

        // 3) Create and/or update the GPU buffers.
        CreateGpuBuffers(vertexLayout, combinedVertexData, combinedIndices, drawCommands_);

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

        vao_->Bind();
        drawCommandBuffer_->Bind();
        glMultiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            nullptr,
            static_cast<GLsizei>(drawCommands_.size()),
            sizeof(DrawElementsIndirectCommand)
        );
        drawCommandBuffer_->Unbind();
        vao_->Unbind();
    }

    void Batch::CullObject(size_t objectIndex) {
        if (objectIndex >= drawCommands_.size()) {
            Logger::GetLogger()->error("Batch::CullObject: objectIndex={} out of range.", objectIndex);
            return;
        }
        auto& cmd = drawCommands_[objectIndex];
        if (cmd.count_ != 0) {
            cmd.count_ = 0;
            std::span<const std::byte> cmdSpan(
                reinterpret_cast<const std::byte*>(&cmd),
                sizeof(DrawElementsIndirectCommand)
            );
            GLintptr offset = objectIndex * sizeof(DrawElementsIndirectCommand);
            drawCommandBuffer_->UpdateData(cmdSpan, offset);
        }
    }

    void Batch::UpdateLOD(size_t objectIndex, size_t newLOD) {
        if (objectIndex >= renderObjects_.size() || objectIndex >= lodInfos_.size()) {
            Logger::GetLogger()->error("Batch::UpdateLOD: invalid objectIndex={}.", objectIndex);
            return;
        }
        auto ro = renderObjects_[objectIndex];
        if (!ro->SetLOD(newLOD))
            return; // No change

        size_t lodUsed = ro->GetCurrentLOD();
        if (lodUsed >= lodInfos_[objectIndex].size())
            lodUsed = 0;
        auto& lodRef = lodInfos_[objectIndex][lodUsed];
        auto& cmd = drawCommands_[objectIndex];
        cmd.count_ = static_cast<GLuint>(lodRef.indexCount_);
        cmd.firstIndex_ = static_cast<GLuint>(lodRef.indexOffsetInCombinedBuffer_);

        std::span<const std::byte> cmdSpan(
            reinterpret_cast<const std::byte*>(&cmd),
            sizeof(DrawElementsIndirectCommand)
        );
        GLintptr offset = objectIndex * sizeof(DrawElementsIndirectCommand);
        drawCommandBuffer_->UpdateData(cmdSpan, offset);
    }

    // ========================= Helper Functions =========================
    // Builds the vertex layout based on the mesh layout and computes totals.
    Batch::BatchGeometryTotals Batch::BuildLayoutAndTotals(VertexBufferLayout& vertexLayout) const {
        BatchGeometryTotals totals;
        GLuint attribIndex = 0;
        if (meshLayout_.hasPositions_) {
            vertexLayout.Push<float>(3, attribIndex++);
            totals.vertexElementCount += 3;
        }
        if (meshLayout_.hasNormals_) {
            vertexLayout.Push<float>(3, attribIndex++);
            totals.vertexElementCount += 3;
        }
        if (meshLayout_.hasTangents_) {
            vertexLayout.Push<float>(3, attribIndex++);
            totals.vertexElementCount += 3;
        }
        if (meshLayout_.hasBitangents_) {
            vertexLayout.Push<float>(3, attribIndex++);
            totals.vertexElementCount += 3;
        }
        for (size_t i = 0; i < meshLayout_.textureTypes_.size(); ++i) {
            if (meshLayout_.textureTypes_.test(i)) {
                vertexLayout.Push<float>(2, attribIndex++);
                totals.vertexElementCount += 2;
            }
        }
        // Sum totals from each render object.
        for (const auto& ro : renderObjects_) {
            totals.totalVertices += ro->GetVertexN();
            totals.totalIndices += ro->GetIndexN();
        }
        return totals;
    }

    // Combines vertex attributes, indices, LOD infos, and draw commands from all RenderObjects.
    void Batch::CombineGeometryData(std::vector<float>& combinedVertexData,
        std::vector<GLuint>& combinedIndices,
        std::vector<std::vector<LODInfo>>& combinedLODInfos,
        std::vector<DrawElementsIndirectCommand>& combinedDrawCommands,
        GLuint& baseVertex) const
    {
        baseVertex = 0;
        for (auto& ro : renderObjects_) {
            auto mesh = ro->GetMesh();
            if (!mesh) {
                Logger::GetLogger()->error("Batch::CombineGeometryData: RenderObject has no valid mesh.");
                continue;
            }
            const size_t vCount = mesh->positions_.size();

            // Append vertex data per vertex.
            for (size_t i = 0; i < vCount; ++i) {
                if (meshLayout_.hasPositions_ && i < mesh->positions_.size()) {
                    const auto& pos = mesh->positions_[i];
                    combinedVertexData.push_back(pos.x);
                    combinedVertexData.push_back(pos.y);
                    combinedVertexData.push_back(pos.z);
                }
                if (meshLayout_.hasNormals_ && i < mesh->normals_.size()) {
                    const auto& nor = mesh->normals_[i];
                    combinedVertexData.push_back(nor.x);
                    combinedVertexData.push_back(nor.y);
                    combinedVertexData.push_back(nor.z);
                }
                if (meshLayout_.hasTangents_ && i < mesh->tangents_.size()) {
                    const auto& tan = mesh->tangents_[i];
                    combinedVertexData.push_back(tan.x);
                    combinedVertexData.push_back(tan.y);
                    combinedVertexData.push_back(tan.z);
                }
                if (meshLayout_.hasBitangents_ && i < mesh->bitangents_.size()) {
                    const auto& bitan = mesh->bitangents_[i];
                    combinedVertexData.push_back(bitan.x);
                    combinedVertexData.push_back(bitan.y);
                    combinedVertexData.push_back(bitan.z);
                }
                // For each texture type.
                for (size_t j = 0; j < meshLayout_.textureTypes_.size(); ++j) {
                    if (meshLayout_.textureTypes_.test(j)) {
                        TextureType texType = static_cast<TextureType>(j);
                        auto uvIt = mesh->uvs_.find(texType);
                        if (uvIt != mesh->uvs_.end() && i < uvIt->second.size()) {
                            const auto& uv = uvIt->second[i];
                            combinedVertexData.push_back(uv.x);
                            combinedVertexData.push_back(uv.y);
                        }
                        else {
                            combinedVertexData.push_back(0.f);
                            combinedVertexData.push_back(0.f);
                        }
                    }
                }
            }

            // Build LOD info and index data for this render object.
            std::vector<LODInfo> objectLODInfos;
            objectLODInfos.reserve(mesh->lods_.size());
            for (const auto& lod : mesh->lods_) {
                LODInfo info;
                info.indexOffsetInCombinedBuffer_ = combinedIndices.size();
                info.indexCount_ = lod.indexCount_;
                for (size_t idx = 0; idx < lod.indexCount_; ++idx) {
                    GLuint oldIndex = mesh->indices_[lod.indexOffset_ + idx];
                    combinedIndices.push_back(oldIndex + baseVertex);
                }
                objectLODInfos.push_back(info);
            }
            combinedLODInfos.push_back(std::move(objectLODInfos));

            // Create the indirect draw command for this object.
            size_t lodUsed = ro->GetCurrentLOD();
            if (lodUsed >= combinedLODInfos.back().size())
                lodUsed = 0;
            auto& usedLOD = combinedLODInfos.back()[lodUsed];
            DrawElementsIndirectCommand cmd{};
            cmd.count_ = static_cast<GLuint>(usedLOD.indexCount_);
            cmd.instanceCount_ = 1;
            cmd.firstIndex_ = static_cast<GLuint>(usedLOD.indexOffsetInCombinedBuffer_);
            cmd.baseVertex_ = 0;
            cmd.baseInstance_ = 0;
            combinedDrawCommands.push_back(cmd);

            baseVertex += static_cast<GLuint>(vCount);
        }
    }

    // Creates or updates the GPU buffers (VBO, IBO, indirect command buffer) and updates the VAO.
    void Batch::CreateGpuBuffers(const VertexBufferLayout& vertexLayout,
        const std::vector<float>& vertexData,
        const std::vector<GLuint>& indexData,
        const std::vector<DrawElementsIndirectCommand>& drawCommands)
    {
        // Create vertex buffer.
        std::span<const std::byte> vertexSpan(
            reinterpret_cast<const std::byte*>(vertexData.data()),
            vertexData.size() * sizeof(float)
        );

        vertexBuffer_ = std::make_unique<VertexBuffer>(vertexSpan, GL_STATIC_DRAW);

        // Create index buffer.
        std::span<const GLuint> indexSpan(indexData.data(), indexData.size());
        indexBuffer_ = std::make_unique<IndexBuffer>(indexSpan, GL_STATIC_DRAW);


        // Create indirect command buffer.
        std::span<const std::byte> cmdSpan(
            reinterpret_cast<const std::byte*>(drawCommands.data()),
            drawCommands.size() * sizeof(DrawElementsIndirectCommand)
        );

        drawCommandBuffer_ = std::make_unique<IndirectBuffer>(cmdSpan, GL_DYNAMIC_DRAW);

        vao_ = std::make_unique<VertexArray>();
        vao_->AddBuffer(*vertexBuffer_, vertexLayout);
        vao_->SetIndexBuffer(*indexBuffer_);
    }

} // namespace Graphics