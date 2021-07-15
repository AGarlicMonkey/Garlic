#pragma once

//There seems to be a bug with optional in msvc that stops it compiling. Having this header included here (above everything) is a current work around.
#include "Clove/Graphics/GhaRenderPass.hpp"
//

#include "Clove/Graphics/GhaBuffer.hpp"
#include "Clove/Graphics/GhaComputePipelineObject.hpp"
#include "Clove/Graphics/GhaComputeQueue.hpp"
#include "Clove/Graphics/GhaDescriptorPool.hpp"
#include "Clove/Graphics/GhaDescriptorSetLayout.hpp"
#include "Clove/Graphics/GhaFence.hpp"
#include "Clove/Graphics/GhaFramebuffer.hpp"
#include "Clove/Graphics/GhaGraphicsPipelineObject.hpp"
#include "Clove/Graphics/GhaGraphicsQueue.hpp"
#include "Clove/Graphics/GhaImage.hpp"
#include "Clove/Graphics/GhaImageView.hpp"
#include "Clove/Graphics/GhaPresentQueue.hpp"
#include "Clove/Graphics/GhaSampler.hpp"
#include "Clove/Graphics/GhaSemaphore.hpp"
#include "Clove/Graphics/GhaShader.hpp"
#include "Clove/Graphics/GhaSwapchain.hpp"
#include "Clove/Graphics/GhaTransferQueue.hpp"
#include "Clove/Graphics/Queue.hpp"

#include <Clove/Expected.hpp>
#include <filesystem>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

namespace garlic::clove {
    /**
     * @brief Used to create graphics objects. Abstracts away the underlying API.
     */
    class GhaFactory {
        //FUNCTIONS
    public:
        virtual ~GhaFactory() = default;

        virtual Expected<std::unique_ptr<GhaGraphicsQueue>, std::runtime_error> createGraphicsQueue(CommandQueueDescriptor descriptor) = 0;
        virtual Expected<std::unique_ptr<GhaPresentQueue>, std::runtime_error> createPresentQueue()                                    = 0;
        virtual Expected<std::unique_ptr<GhaTransferQueue>, std::runtime_error> createTransferQueue(CommandQueueDescriptor descriptor) = 0;
        virtual Expected<std::unique_ptr<GhaComputeQueue>, std::runtime_error> createComputeQueue(CommandQueueDescriptor descriptor)   = 0;

        virtual Expected<std::unique_ptr<GhaSwapchain>, std::runtime_error> createSwapChain(GhaSwapchain::Descriptor descriptor) = 0;

        /**
         * @brief Compile a GLSL file and return the GhaShader object.
         * @param file Absolute path to the file.
         * @param shaderStage Which shader stage to compile.
         * @return Compiled shader module.
         */
        virtual Expected<std::unique_ptr<GhaShader>, std::runtime_error> createShaderFromFile(std::filesystem::path const &file, GhaShader::Stage shaderStage) = 0;
        /**
         * @brief Compile GLSL source code and return the GhaShader object
         * @param includeSources std::unordered map of shader source strings to use as includes. Key is the name of the include and value is the string of the include.
         * @param shaderName The name of the shader being compiled. Used for debug purposes, can be empty.
         * @return Compiled shader module.
         */
        virtual Expected<std::unique_ptr<GhaShader>, std::runtime_error> createShaderFromSource(std::string_view source, std::unordered_map<std::string, std::string> includeSources, std::string_view shaderName, GhaShader::Stage shaderStage) = 0;

        virtual Expected<std::unique_ptr<GhaRenderPass>, std::runtime_error> createRenderPass(GhaRenderPass::Descriptor descriptor)                            = 0;
        virtual Expected<std::unique_ptr<GhaDescriptorSetLayout>, std::runtime_error> createDescriptorSetLayout(GhaDescriptorSetLayout::Descriptor descriptor) = 0;

        virtual Expected<std::unique_ptr<GhaGraphicsPipelineObject>, std::runtime_error> createGraphicsPipelineObject(GhaGraphicsPipelineObject::Descriptor descriptor) = 0;
        virtual Expected<std::unique_ptr<GhaComputePipelineObject>, std::runtime_error> createComputePipelineObject(GhaComputePipelineObject::Descriptor descriptor)    = 0;

        virtual Expected<std::unique_ptr<GhaFramebuffer>, std::runtime_error> createFramebuffer(GhaFramebuffer::Descriptor descriptor)          = 0;
        virtual Expected<std::unique_ptr<GhaDescriptorPool>, std::runtime_error> createDescriptorPool(GhaDescriptorPool::Descriptor descriptor) = 0;

        virtual Expected<std::unique_ptr<GhaSemaphore>, std::runtime_error> createSemaphore()                        = 0;
        virtual Expected<std::unique_ptr<GhaFence>, std::runtime_error> createFence(GhaFence::Descriptor descriptor) = 0;

        virtual Expected<std::unique_ptr<GhaBuffer>, std::runtime_error> createBuffer(GhaBuffer::Descriptor descriptor) = 0;

        virtual Expected<std::unique_ptr<GhaImage>, std::runtime_error> createImage(GhaImage::Descriptor descriptor) = 0;
        /**
         * @brief Creates a view of a GhaImage that can be used in shaders.
         * @param image The image to create a view from.
         * @param descriptor 
         * @return 
         */
        virtual Expected<std::unique_ptr<GhaImageView>, std::runtime_error> createImageView(GhaImage const &image, GhaImageView::Descriptor descriptor) = 0;

        virtual Expected<std::unique_ptr<GhaSampler>, std::runtime_error> createSampler(GhaSampler::Descriptor descriptor) = 0;
    };
}