#pragma once

#include "Clove/Graphics/GhaRenderPass.hpp"

#include <MetalKit/MetalKit.h>

namespace garlic::clove {
	class MetalRenderPass : public GhaRenderPass {
		//VARIABLES
	private:
		Descriptor descriptor{};

		MTLRenderPipelineColorAttachmentDescriptorArray* colourAttachments{ nullptr };
		MTLPixelFormat depthPixelFormat;
		
		//FUNCTIONS
	public:
		MetalRenderPass() = delete;
        MetalRenderPass(Descriptor descriptor, MTLRenderPipelineColorAttachmentDescriptorArray *colourAttachments, MTLPixelFormat depthPixelFormat);

        MetalRenderPass(MetalRenderPass const &other) = delete;
		MetalRenderPass(MetalRenderPass &&other) noexcept;
		
		MetalRenderPass& operator=(MetalRenderPass const &other) = delete;
		MetalRenderPass& operator=(MetalRenderPass &&other) noexcept;
		
		~MetalRenderPass();
		
		Descriptor const &getDescriptor() const override;

		inline MTLRenderPipelineColorAttachmentDescriptorArray* getColourAttachments() const;
		inline MTLPixelFormat getDepthPixelFormat() const;
	};
}

#include "MetalRenderPass.inl"
