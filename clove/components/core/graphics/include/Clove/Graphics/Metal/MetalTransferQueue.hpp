#pragma once

#include "Clove/Graphics/GhaTransferQueue.hpp"
#include "Clove/Graphics/Queue.hpp"

#include <MetalKit/MetalKit.h>

namespace garlic::clove {
	class MetalTransferQueue : public GhaTransferQueue {
		//VARIABLES
	private:
		id<MTLCommandQueue> commandQueue;
		
		bool allowBufferReuse{ false };
		
		//FUNCTIONS
	public:
		MetalTransferQueue() = delete;
		MetalTransferQueue(CommandQueueDescriptor descriptor, id<MTLCommandQueue> commandQueue);
		
		MetalTransferQueue(MetalTransferQueue const &other) = delete;
		MetalTransferQueue(MetalTransferQueue &&other) noexcept;
		
		MetalTransferQueue& operator=(MetalTransferQueue const &other) = delete;
		MetalTransferQueue& operator=(MetalTransferQueue &&other) noexcept;
		
		~MetalTransferQueue();

		std::unique_ptr<GhaTransferCommandBuffer> allocateCommandBuffer() override;
		void freeCommandBuffer(GhaTransferCommandBuffer &buffer) override;

		void submit(std::vector<TransferSubmitInfo> const &submissions, GhaFence *signalFence) override;
	};
}

