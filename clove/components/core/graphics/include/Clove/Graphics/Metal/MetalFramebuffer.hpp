#pragma once

#include "Clove/Graphics/GhaFramebuffer.hpp"

#include <MetalKit/MetalKit.h>

namespace garlic::clove {
	class MetalFramebuffer : public GhaFramebuffer {
		//VARIABLES
	private:
		MTLRenderPassDescriptor *renderPassDescriptor{ nullptr };
		
		//FUNCTIONS
	public:
		MetalFramebuffer() = delete;
		MetalFramebuffer(MTLRenderPassDescriptor *renderPassDescriptor);
		
		MetalFramebuffer(MetalFramebuffer const &other) = delete;
		MetalFramebuffer(MetalFramebuffer &&other) noexcept;
		
		MetalFramebuffer& operator=(MetalFramebuffer const &other) = delete;
		MetalFramebuffer& operator=(MetalFramebuffer &&other) noexcept;
		
		~MetalFramebuffer();
		
		MTLRenderPassDescriptor *getRenderPassDescriptor() const;
	};
}
