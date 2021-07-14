#pragma once

#include "Clove/Graphics/GhaSemaphore.hpp"

#include <MetalKit/MetalKit.h>

namespace garlic::clove {
	class MetalSemaphore : public GhaSemaphore {
		//VARIABLES
	private:
		//In metal fences and semaphores are swapped. So a fence synchronises between MTLCommandBuffers
		id<MTLFence> fence{ nullptr };
		
		//FUNCTIONS
	public:
		MetalSemaphore() = delete;
		MetalSemaphore(id<MTLFence> fence);
		
		MetalSemaphore(MetalSemaphore const &other) = delete;
		MetalSemaphore(MetalSemaphore &&other) noexcept;
		
		MetalSemaphore &operator=(MetalSemaphore const &other) = delete;
		MetalSemaphore &operator=(MetalSemaphore &&other) noexcept;
		
		~MetalSemaphore();
		
		inline id<MTLFence> getFence() const;
	};
}

#include "MetalSemaphore.inl"
