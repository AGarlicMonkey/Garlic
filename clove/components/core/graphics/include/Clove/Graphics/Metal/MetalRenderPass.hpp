#pragma once

#include "Clove/Graphics/GhaRenderPass.hpp"

#include <MetalKit/MetalKit.h>

namespace garlic::clove {
	class MetalRenderPass : public GhaRenderPass {
		//VARIABLES
	private:
		MTLRenderPipelineColorAttachmentDescriptorArray* colourAttachments{ nullptr };
		MTLPixelFormat depthPixelFormat;
		
		Descriptor descriptor{};
		
		//FUNCTIONS
	public:
		MetalRenderPass() = delete;
		MetalRenderPass(MTLRenderPipelineColorAttachmentDescriptorArray* colourAttachments, MTLPixelFormat depthPixelFormat, Descriptor descriptor);
		
		MetalRenderPass(MetalRenderPass const &other) = delete;
		MetalRenderPass(MetalRenderPass &&other) noexcept;
		
		MetalRenderPass& operator=(MetalRenderPass const &other) = delete;
		MetalRenderPass& operator=(MetalRenderPass &&other) noexcept;
		
		~MetalRenderPass();
		
		inline MTLRenderPipelineColorAttachmentDescriptorArray* getColourAttachments() const;
		inline MTLPixelFormat getDepthPixelFormat() const;
		
		inline Descriptor const &getDescriptor() const;
	};
}

#include "MetalRenderPass.inl"
