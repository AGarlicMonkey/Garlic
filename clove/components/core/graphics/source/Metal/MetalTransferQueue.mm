#include "Clove/Graphics/Metal/MetalTransferQueue.hpp"

#include "Clove/Graphics/Metal/MetalPipelineObject.hpp"
#include "Clove/Graphics/Metal/MetalTransferCommandBuffer.hpp"
#include "Clove/Graphics/Metal/MetalSemaphore.hpp"
#include "Clove/Graphics/Metal/MetalFence.hpp"

#include <Clove/Cast.hpp>

namespace garlic::clove {
	MetalTransferQueue::MetalTransferQueue(CommandQueueDescriptor descriptor, id<MTLCommandQueue> commandQueue)
		: commandQueue{ [commandQueue retain] } {
		allowBufferReuse = (descriptor.flags & QueueFlags::ReuseBuffers) != 0;
	}
	
	MetalTransferQueue::MetalTransferQueue(MetalTransferQueue &&other) noexcept = default;
	
	MetalTransferQueue& MetalTransferQueue::operator=(MetalTransferQueue &&other) noexcept = default;
	
	MetalTransferQueue::~MetalTransferQueue() {
		[commandQueue release];
	}

	std::unique_ptr<GhaTransferCommandBuffer> MetalTransferQueue::allocateCommandBuffer() {
		return std::make_unique<MetalTransferCommandBuffer>(allowBufferReuse);
	}
	
	void MetalTransferQueue::freeCommandBuffer(GhaTransferCommandBuffer &buffer) {
		//no op
	}

	void MetalTransferQueue::submit(std::vector<TransferSubmitInfo> const &submissions, GhaFence *signalFence) {
		@autoreleasepool {
			for(auto const &submission : submissions) {
				for(auto const &commandBuffer : submission.commandBuffers) {
					auto *metalCommandBuffer{ polyCast<MetalTransferCommandBuffer>(commandBuffer.get()) };
					if(metalCommandBuffer->getCommandBufferUsage() == CommandBufferUsage::OneTimeSubmit && metalCommandBuffer->bufferHasBeenUsed()){
						CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Error, "TransferCommandBuffer recorded with CommandBufferUsage::OneTimeSubmit has already been used. Only buffers recorded with CommandBufferUsage::Default can submitted multiples times after being recorded once.");
						break;
					}
					
					id<MTLCommandBuffer> executionBuffer{ [commandQueue commandBuffer] };
					id<MTLBlitCommandEncoder> encoder{ [executionBuffer blitCommandEncoder] };
					
					//Inject the wait semaphore into each buffer
					for (auto const &semaphore : submission.waitSemaphores) {
						auto *metalSemaphore{ polyCast<MetalSemaphore const>(semaphore.first.get()) };
						
						[encoder waitForFence:metalSemaphore->getFence()];
					}
					
					//Excute all recorded commands for the encoder
					for(auto &command : metalCommandBuffer->getCommands()) {
						command(encoder);
					}
					
					[encoder endEncoding];
					[executionBuffer commit];
					
					metalCommandBuffer->markAsUsed();
				}
				
				//Signal all semaphores at the end of each submission. Similar to how vulkan does it.
				if(!submission.signalSemaphores.empty()) {
					id<MTLCommandBuffer> signalBuffer{ [commandQueue commandBuffer] };
					id<MTLBlitCommandEncoder> encoder{ [signalBuffer blitCommandEncoder] };
					
					for(auto const &semaphore : submission.signalSemaphores) {
						[encoder updateFence:polyCast<MetalSemaphore const>(semaphore.get())->getFence()];
					}
					
					[encoder endEncoding];
					[signalBuffer commit];
				}
			}
			
			//Signal fence when all submissions are completed
			if(signalFence != nullptr) {
				id<MTLCommandBuffer> signalBuffer{ [commandQueue commandBuffer] };
				
				__block auto *fence{ polyCast<MetalFence>(signalFence) };
				[signalBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
					fence->signal();
				}];
				
				[signalBuffer commit];
			}
		}
	}
}
