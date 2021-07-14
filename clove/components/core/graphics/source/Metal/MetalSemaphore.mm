#include "Clove/Graphics/Metal/MetalSemaphore.hpp"

namespace garlic::clove {
	MetalSemaphore::MetalSemaphore(id<MTLFence> fence)
		: fence{ [fence retain] } {
	}
	
	MetalSemaphore::MetalSemaphore(MetalSemaphore &&other) noexcept = default;
	
	MetalSemaphore &MetalSemaphore::operator=(MetalSemaphore &&other) noexcept = default;
	
	MetalSemaphore::~MetalSemaphore() {
		[fence release];
	}
}
