#pragma once

#include "Clove/Graphics/GhaPresentQueue.hpp"

#include <MetalKit/MetalKit.h>

@class MetalView;

namespace garlic::clove {
	class MetalPresentQueue : public GhaPresentQueue {
		//VARIABLES
	private:
		id<MTLCommandQueue> commandQueue;
		MetalView *view{ nullptr };
		
		//FUNCTIONS
	public:
		MetalPresentQueue() = delete;
		MetalPresentQueue(id<MTLCommandQueue> commandQueue, MetalView *view);
		
		MetalPresentQueue(MetalPresentQueue const &other) = delete;
		MetalPresentQueue(MetalPresentQueue &&other) noexcept;
		
		MetalPresentQueue &operator=(MetalPresentQueue const &other) = delete;
		MetalPresentQueue &operator=(MetalPresentQueue &&other) noexcept;
		
		~MetalPresentQueue();
		
		Result present(PresentInfo const &presentInfo) override;
	};
}
