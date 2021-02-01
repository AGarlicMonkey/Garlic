#pragma once

#include "Clove/Graphics/DescriptorPool.hpp"
#include "Clove/Graphics/DescriptorSetLayout.hpp"
#include "Clove/Graphics/Fence.hpp"
#include "Clove/Graphics/Framebuffer.hpp"
#include "Clove/Graphics/GraphicsBuffer.hpp"
#include "Clove/Graphics/GraphicsImage.hpp"
#include "Clove/Graphics/GraphicsQueue.hpp"
#include "Clove/Graphics/PipelineObject.hpp"
#include "Clove/Graphics/PresentQueue.hpp"
#include "Clove/Graphics/RenderPass.hpp"
#include "Clove/Graphics/Sampler.hpp"
#include "Clove/Graphics/Semaphore.hpp"
#include "Clove/Graphics/Shader.hpp"
#include "Clove/Graphics/Swapchain.hpp"
#include "Clove/Graphics/TransferQueue.hpp"

#include <Clove/Expected.hpp>
#include <filesystem>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

namespace garlic::clove {
    /**
     * @brief Used to create graphics objects. Abstracts away the underlying API.
     */
    class GraphicsFactory {
        //FUNCTIONS
    public:
        virtual ~GraphicsFactory() = default;

        virtual Expected<std::unique_ptr<GraphicsQueue>, std::runtime_error> createGraphicsQueue(CommandQueueDescriptor descriptor) = 0;
        virtual Expected<std::unique_ptr<PresentQueue>, std::runtime_error> createPresentQueue()                                    = 0;
        virtual Expected<std::unique_ptr<TransferQueue>, std::runtime_error> createTransferQueue(CommandQueueDescriptor descriptor) = 0;

        virtual Expected<std::unique_ptr<Swapchain>, std::runtime_error> createSwapChain(Swapchain::Descriptor descriptor) = 0;

        /**
         * @brief Compile a GLSL file and return the Shader object.
         * @param file Absolute path to the file.
         * @param shaderStage Which shader stage to compile.
         * @return Compiled shader module.
         */
        virtual Expected<std::unique_ptr<Shader>, std::runtime_error> createShaderFromFile(std::filesystem::path const &file, Shader::Stage shaderStage) = 0;
        /**
         * @brief Compile GLSL source code and return the Shader object
         * @param includeSources std::unordered map of shader source strings to use as includes. Key is the name of the include and value is the string of the include.
         * @param shaderName The name of the shader being compiled. Used for debug purposes, can be empty.
         * @return Compiled shader module.
         */
        virtual Expected<std::unique_ptr<Shader>, std::runtime_error> createShaderFromSource(std::string_view source, std::unordered_map<std::string, std::string> includeSources, std::string_view shaderName, Shader::Stage shaderStage) = 0;

        virtual Expected<std::unique_ptr<RenderPass>, std::runtime_error> createRenderPass(RenderPass::Descriptor descriptor)                            = 0;
        virtual Expected<std::unique_ptr<DescriptorSetLayout>, std::runtime_error> createDescriptorSetLayout(DescriptorSetLayout::Descriptor descriptor) = 0;

        virtual Expected<std::unique_ptr<PipelineObject>, std::runtime_error> createPipelineObject(PipelineObject::Descriptor descriptor) = 0;

        virtual Expected<std::unique_ptr<Framebuffer>, std::runtime_error> createFramebuffer(Framebuffer::Descriptor descriptor)          = 0;
        virtual Expected<std::unique_ptr<DescriptorPool>, std::runtime_error> createDescriptorPool(DescriptorPool::Descriptor descriptor) = 0;

        virtual Expected<std::unique_ptr<Semaphore>, std::runtime_error> createSemaphore()                     = 0;
        virtual Expected<std::unique_ptr<Fence>, std::runtime_error> createFence(Fence::Descriptor descriptor) = 0;

        virtual Expected<std::unique_ptr<GraphicsBuffer>, std::runtime_error> createBuffer(GraphicsBuffer::Descriptor descriptor) = 0;
        virtual Expected<std::unique_ptr<GraphicsImage>, std::runtime_error> createImage(GraphicsImage::Descriptor descriptor)    = 0;

        virtual Expected<std::unique_ptr<Sampler>, std::runtime_error> createSampler(Sampler::Descriptor descriptor) = 0;
    };
}