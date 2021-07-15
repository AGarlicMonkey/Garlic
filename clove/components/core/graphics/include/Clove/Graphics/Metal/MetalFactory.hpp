#pragma once

#include "Clove/Graphics/GhaFactory.hpp"

#include <MetalKit/MetalKit.h>
#include <span>

@class MetalView;

namespace garlic::clove {
	class MetalFactory : public GhaFactory {
		//VARIABLES
	private:
		id<MTLDevice> device{ nullptr };
		MetalView *view{ nullptr };
		
		id<MTLCommandQueue> commandQueue{ nullptr };
		
		//FUNCTIONS
	public:
		MetalFactory() = delete;
		MetalFactory(id<MTLDevice> device, MetalView *view);

		MetalFactory(MetalFactory const &other) = delete;
		MetalFactory(MetalFactory &&other) noexcept;

		MetalFactory &operator=(MetalFactory const &other) = delete;
		MetalFactory &operator=(MetalFactory &&other) noexcept;

		~MetalFactory();

		Expected<std::unique_ptr<GhaGraphicsQueue>, std::runtime_error> createGraphicsQueue(CommandQueueDescriptor descriptor) override;
		Expected<std::unique_ptr<GhaPresentQueue>, std::runtime_error> createPresentQueue() override;
		Expected<std::unique_ptr<GhaTransferQueue>, std::runtime_error> createTransferQueue(CommandQueueDescriptor descriptor) override;
		Expected<std::unique_ptr<GhaComputeQueue>, std::runtime_error> createComputeQueue(CommandQueueDescriptor descriptor) override;
		
		Expected<std::unique_ptr<GhaSwapchain>, std::runtime_error> createSwapChain(GhaSwapchain::Descriptor descriptor) override;

		Expected<std::unique_ptr<GhaShader>, std::runtime_error> createShaderFromFile(std::filesystem::path const &file, GhaShader::Stage shaderStage) override;
		Expected<std::unique_ptr<GhaShader>, std::runtime_error> createShaderFromSource(std::string_view source, std::unordered_map<std::string, std::string> includeSources, std::string_view shaderName, GhaShader::Stage shaderStage) override;

		Expected<std::unique_ptr<GhaRenderPass>, std::runtime_error> createRenderPass(GhaRenderPass::Descriptor descriptor) override;
		Expected<std::unique_ptr<GhaDescriptorSetLayout>, std::runtime_error> createDescriptorSetLayout(GhaDescriptorSetLayout::Descriptor descriptor) override;

		Expected<std::unique_ptr<GhaGraphicsPipelineObject>, std::runtime_error> createGraphicsPipelineObject(GhaGraphicsPipelineObject::Descriptor descriptor) override;
		Expected<std::unique_ptr<GhaComputePipelineObject>, std::runtime_error> createComputePipelineObject(GhaComputePipelineObject::Descriptor descriptor) override;
		
		Expected<std::unique_ptr<GhaFramebuffer>, std::runtime_error> createFramebuffer(GhaFramebuffer::Descriptor descriptor) override;
		Expected<std::unique_ptr<GhaDescriptorPool>, std::runtime_error> createDescriptorPool(GhaDescriptorPool::Descriptor descriptor) override;

		Expected<std::unique_ptr<GhaSemaphore>, std::runtime_error> createSemaphore() override;
		Expected<std::unique_ptr<GhaFence>, std::runtime_error> createFence(GhaFence::Descriptor descriptor) override;

		Expected<std::unique_ptr<GhaBuffer>, std::runtime_error> createBuffer(GhaBuffer::Descriptor descriptor) override;

		Expected<std::unique_ptr<GhaImage>, std::runtime_error> createImage(GhaImage::Descriptor descriptor) override;
        Expected<std::unique_ptr<GhaImageView>, std::runtime_error> createImageView(GhaImage const &image, GhaImageView::Descriptor descriptor) override;
		
        Expected<std::unique_ptr<GhaSampler>, std::runtime_error> createSampler(GhaSampler::Descriptor descriptor) override;
		
	private:
        Expected<std::unique_ptr<GhaShader>, std::runtime_error> createShaderObject(std::span<uint32_t> spirvSource);
    };
}
