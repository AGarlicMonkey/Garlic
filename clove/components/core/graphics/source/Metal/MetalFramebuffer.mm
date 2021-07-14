#include "Clove/Graphics/Metal/MetalFramebuffer.hpp"

namespace garlic::clove {
	MetalFramebuffer::MetalFramebuffer(MTLRenderPassDescriptor *renderPassDescriptor)
		: renderPassDescriptor{ [renderPassDescriptor retain] } {
	}
	
	MetalFramebuffer::MetalFramebuffer(MetalFramebuffer &&other) noexcept = default;
	
	MetalFramebuffer& MetalFramebuffer::operator=(MetalFramebuffer &&other) noexcept = default;
	
	MetalFramebuffer::~MetalFramebuffer() {
		[renderPassDescriptor release];
	}
	
	MTLRenderPassDescriptor *MetalFramebuffer::getRenderPassDescriptor() const {
		return renderPassDescriptor;
	}
}
