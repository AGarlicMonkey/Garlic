#pragma once

#include "Clove/Graphics/GhaGraphicsQueue.hpp"
#include "Clove/Graphics/Queue.hpp"

#include <MetalKit/MetalKit.h>

@class MetalView;

namespace garlic::clove {
	class MetalGraphicsQueue : public GhaGraphicsQueue {
		//VARIABLES
	private:
		id<MTLCommandQueue> commandQueue;
		
		bool allowBufferReuse{ false };

		//FUNCTIONS
	public:
		MetalGraphicsQueue() = delete;
		MetalGraphicsQueue(CommandQueueDescriptor descriptor, id<MTLCommandQueue> commandQueue);
		
		MetalGraphicsQueue(MetalGraphicsQueue const &other) = delete;
		MetalGraphicsQueue(MetalGraphicsQueue &&other) noexcept;
		
		MetalGraphicsQueue& operator=(MetalGraphicsQueue const &other) = delete;
		MetalGraphicsQueue& operator=(MetalGraphicsQueue &&other) noexcept;
		
		~MetalGraphicsQueue();

		std::unique_ptr<GhaGraphicsCommandBuffer> allocateCommandBuffer() override;
		void freeCommandBuffer(GhaGraphicsCommandBuffer &buffer) override;

		void submit(std::vector<GraphicsSubmitInfo> const &submissions, GhaFence *signalFence) override;
	};
}

