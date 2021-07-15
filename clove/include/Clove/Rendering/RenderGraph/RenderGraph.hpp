#pragma once

#include "Clove/Rendering/RenderGraph/RgBuffer.hpp"
#include "Clove/Rendering/RenderGraph/RgImage.hpp"
#include "Clove/Rendering/RenderGraph/RgRenderPass.hpp"
#include "Clove/Rendering/RenderGraph/RgTransferPass.hpp"
#include "Clove/Rendering/RenderGraph/RgShader.hpp"

#include <Clove/Graphics/GhaGraphicsQueue.hpp>
#include <Clove/Graphics/GhaShader.hpp>
#include <Clove/Maths/Vector.hpp>
#include <filesystem>
#include <functional>
#include <unordered_map>
#include <vector>

namespace garlic::clove {
    class RgFrameCache;
    class RgGlobalCache;

    class GhaFactory;
    class GhaComputeQueue;
    class GhaGraphicsQueue;
    class GhaTransferQueue;
    class GhaGraphicsCommandBuffer;
    class GhaComputeCommandBuffer;
    class GhaTransferCommandBuffer;
}

namespace garlic::clove {
    /**
     * @brief Tracks how resources are used to create a graph of how to render a frame.
     */
    class RenderGraph {
        //VARIABLES
    private:
        RgFrameCache &frameCache;
        RgGlobalCache &globalCache;

        RgResourceIdType nextResourceId{ 1 };
        RgPassIdType nextPassId{ 1 };

        RgResourceIdType outputResource{ INVALID_RESOURCE_ID }; /**< The final output of the graph. */

        std::unordered_map<RgResourceIdType, std::unique_ptr<RgBuffer>> buffers{};
        std::unordered_map<RgResourceIdType, std::unique_ptr<RgImage>> images{};

        std::unordered_map<RgPassIdType, std::unique_ptr<RgRenderPass>> renderPasses{};
        std::unordered_map<RgPassIdType, std::unique_ptr<RgTransferPass>> transferPasses{};

        std::unordered_map<RgResourceIdType, std::shared_ptr<GhaShader>> allocatedShaders{};

        //FUNCTIONS
    public:
        RenderGraph() = delete;
        RenderGraph(RgFrameCache &frameCache, RgGlobalCache &globalCache);

        RenderGraph(RenderGraph const &other)     = delete;
        RenderGraph(RenderGraph &&other) noexcept = delete;

        RenderGraph &operator=(RenderGraph const &other) = delete;
        RenderGraph &operator=(RenderGraph &&other) noexcept = delete;

        ~RenderGraph();

        /**
         * @brief Constructs an RgBuffer with the specified size.
         * @details Buffers are created in video memory unless written to from the CPU.
         * @param bufferSize 
         * @return 
         */
        RgResourceIdType createBuffer(size_t const bufferSize);
        /**
         * @brief Constructs an RgBuffer from an existing GhaBuffer. Usefull if wanting to use
         * pre set up buffers (such as vertex / index buffers).
         * @param buffer GhaBuffer to construct from.
         * @param offset The offset into the GhaBuffer that this RgBuffer will view.
         * @param size The size of the GhaBuffer that this RgBuffer will view.
         * @return
         */
        RgResourceIdType createBuffer(std::shared_ptr<GhaBuffer> buffer, size_t const offset, size_t const size);

        /**
         * @brief Write data into this buffer.
         * @details Will transition this buffer from video memory to system memory if requierd.
         * @param data Pointer to data to write.
         * @param offset Offset into the buffer to write to. If the RgBuffer views an offset into a GhaBuffer then it'll be view offset + offset
         * @param size Size of the region in the buffer to write to.
         */
        void writeToBuffer(RgResourceIdType buffer, void const *data, size_t const offset, size_t const size);

        /**
         * @brief Constructs a new RgImage with the specified type and dimensions.
         * @param imageType 
         * @param format
         * @param dimensions 
         * @return 
         */
        RgResourceIdType createImage(GhaImage::Type imageType, GhaImage::Format format, vec2ui dimensions);
        /**
         * @brief Creates a new RgImage from an existing image view. Usefull
         * if wanting to use pre made images (such as backbuffers) in the render graph.
         * @param ghaImageView 
         * @return 
         */
        RgResourceIdType createImage(std::shared_ptr<GhaImageView> ghaImageView);

        /**
         * @brief Registers a resource as the final output of the graph.
         * @details The graph will be traversed starting from this resource and build
         * the list of passes it will execute bassed on this output's dependencies.
         * @param resource 
         */
        void registerGraphOutput(RgResourceIdType resource);

        /**
         * @brief Creates a new RgShader from a source file.
         * @param file 
         * @param shaderStage 
         * @return 
         */
        RgShader createShader(std::filesystem::path const &file, GhaShader::Stage shaderStage);
        /**
         * @brief Create a new RgShader from a source string.
         * @param source 
         * @param includeSources A map of source strings that will get searched for any #includes.
         * @param shaderName A name for the shader being compiled.
         * @param shaderStage 
         * @return 
         */
        RgShader createShader(std::string_view source, std::unordered_map<std::string, std::string> includeSources, std::string_view shaderName, GhaShader::Stage shaderStage);

        /**
         * @brief Add a render pass into the graph.
         * @param passDescriptor Descriptor of the pass itself. Contains any output resources and what shaders to run.
         */
        RgPassIdType addRenderPass(RgRenderPass::Descriptor passDescriptor);
        /**
         * @brief Adds a submission to a render pass. Defining a piece of geometry it needs to render.
         * @param renderPass Pass to add the submission to.
         * @param submission Submission for the pass. Encapsulates any input resources required for a single unit of work.
         */
        void addRenderSubmission(RgPassIdType const renderPass, RgRenderPass::Submission submission);

        /**
         * @brief Executes the RenderGraph. Creating any objects required and submitting commands to the relevant queues.
         * @param factory Requires a factory to create any GHA objects when traversing the graph.
         * @param graphicsQueue The queue the graph will submit graphics work to.
         * @param computeQueue The queue the graph will submit compute work to.
         * @param transferQueue The queue the graph will submit transfer work to.
         * @return Returns the GraphicsSubmitInfo used to render the final result of the graph.
         */
        GraphicsSubmitInfo execute();

    private:
        void buildExecutionPasses(std::vector<RgPassIdType> &passes, RgResourceIdType resourceId);
        GhaImage::Layout getPreviousLayout(std::vector<RgPassIdType> const &passes, int32_t const currentPassIndex, RgResourceIdType const imageId);

        RgResource *getResourceFromId(RgResourceIdType resourceId);
        RgPass *getPassFromId(RgPassIdType passId);
    };
}