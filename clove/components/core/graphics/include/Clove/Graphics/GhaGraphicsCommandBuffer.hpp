#pragma once

#include "Clove/Graphics/CommandBuffer.hpp"
#include "Clove/Graphics/GhaShader.hpp"
#include "Clove/Graphics/MemoryBarrier.hpp"
#include "Clove/Graphics/PipelineObject.hpp"

#include <Clove/Maths/Vector.hpp>
#include <span>
#include <variant>

namespace garlic::clove {
    class GhaRenderPass;
    class GhaFramebuffer;
    class GhaGraphicsPipelineObject;
    class GhaBuffer;
    class GhaDescriptorSet;
    class GhaImage;
    class GhaBuffer;
}

namespace garlic::clove {
    struct RenderArea {
        vec2i origin;
        vec2ui size;
    };

    using ColourValue = vec4f;

    struct DepthStencilValue {
        float depth{ 0.0f };
        uint32_t stencil{ 0 };
    };

    using ClearValue = std::variant<ColourValue, DepthStencilValue>;

    enum class IndexType {
        Uint16
    };
}

namespace garlic::clove {
    /**
     * @brief A buffer for recording graphics commands.
     * @details Allocated from a GhaGraphicsQueue.
     */
    class GhaGraphicsCommandBuffer {
        //FUNCTIONS
    public:
        virtual ~GhaGraphicsCommandBuffer() = default;

        virtual void beginRecording(CommandBufferUsage usageFlag) = 0;
        virtual void endRecording()                               = 0;

        /**
         * @details Begins a GhaRenderPass. All subsiquent calls will use this render pass.
         * @param renderPass The GhaRenderPass to begin.
         * @param frameBuffer The FrameBuffer to use.
         * @param renderArea What area to render onto the frameBuffer.
         * @param clearValues An array of clear values. Each element in the array represents an attachment in the frameBuffer.
         */
        virtual void beginRenderPass(GhaRenderPass &renderPass, GhaFramebuffer &frameBuffer, RenderArea const &renderArea, std::span<ClearValue> clearValues) = 0;
        virtual void endRenderPass()                                                                                                                          = 0;

        virtual void setViewport(vec2i position, vec2ui size) = 0;
        virtual void setScissor(vec2i position, vec2ui size)  = 0;

        virtual void bindPipelineObject(GhaGraphicsPipelineObject &pipelineObject) = 0;
        /**
         * @brief Bind a vertex buffer to be used in the next draw call.
         * @param offset Offset into the buffer where the vertices begin.
         */
        virtual void bindVertexBuffer(GhaBuffer &vertexBuffer, size_t const offset) = 0;
        /**
         * @brief Bind an index buffer to be used in the next draw call.
         * @param offset Offset into the buffer where the indices begin.
         */
        virtual void bindIndexBuffer(GhaBuffer &indexBuffer, size_t const offset, IndexType indexType) = 0;

        /**
         * @brief Bind a descriptor set. Making all written data available to the pipeline.
         * @param descriptorSet 
         * @param setNum Which set to upload the descriptor set into.
         */
        virtual void bindDescriptorSet(GhaDescriptorSet &descriptorSet, uint32_t const setNum) = 0;
        /**
         * @brief Uploads a small amount of data to the pipeline without the need for descriptor sets.
         * @details A single block of push constant range is shared across all shader stages. When pushing
         * multiple values the offsets and sizes will need to line up.
         * @param stage Which shader stage to upload to.
         * @param offset The offset into the push constant range to upload to.
         * @param size The size of the push constant range to update.
         * @param data The data to upload.
         */
        virtual void pushConstant(GhaShader::Stage const stage, size_t const offset, size_t const size, void const *data) = 0;

        virtual void drawIndexed(size_t const indexCount) = 0;

        /**
         * @brief Creates a memory barrier for a buffer. Allowing for how it's accessed to be changed and/or to transfer queue ownership.
         * @param buffer The buffer to create the barrier for.
         * @param barrierInfo The information about the barrier.
         * @param sourceStage The pipeline stage that gets executed before the barrier.
         * @param destinationStage The pipeline stage executed after the barrier that waits for the results of the sourceStage.
         */
        virtual void bufferMemoryBarrier(GhaBuffer &buffer, BufferMemoryBarrierInfo const &barrierInfo, PipelineStage sourceStage, PipelineStage destinationStage) = 0;
        /**
         * @brief Creates a memory barrier for an image. Allowing for how it's accessed, it's layout and queue ownership to change.
         * @param image The image to create the barrier for.
         * @param barrierInfo The information about the barrier.
         * @param sourceStage The pipeline stage that gets executed before the barrier.
         * @param destinationStage The pipeline stage executed after the barrier that waits for the results of the sourceStage.
         */
        virtual void imageMemoryBarrier(GhaImage &image, ImageMemoryBarrierInfo const &barrierInfo, PipelineStage sourceStage, PipelineStage destinationStage) = 0;
    };
}