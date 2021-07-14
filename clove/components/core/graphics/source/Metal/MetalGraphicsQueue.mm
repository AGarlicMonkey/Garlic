#include "Clove/Graphics/Metal/MetalGraphicsQueue.hpp"

#include "Clove/Graphics/Metal/MetalGraphicsCommandBuffer.hpp"
#include "Clove/Graphics/Metal/MetalView.hpp"
#include "Clove/Graphics/Metal/MetalFence.hpp"
#include "Clove/Graphics/Metal/MetalPipelineObject.hpp"
#include "Clove/Graphics/Metal/MetalSemaphore.hpp"

#include <Clove/Cast.hpp>

namespace garlic::clove {
	MetalGraphicsQueue::MetalGraphicsQueue(clove::CommandQueueDescriptor descriptor, id<MTLCommandQueue> commandQueue)
		: commandQueue{ [commandQueue retain] } {
		allowBufferReuse = (descriptor.flags & QueueFlags::ReuseBuffers) != 0;
	}
	
	MetalGraphicsQueue::MetalGraphicsQueue(MetalGraphicsQueue &&other) noexcept = default;
	
	MetalGraphicsQueue& MetalGraphicsQueue::operator=(MetalGraphicsQueue &&other) noexcept = default;
	
	MetalGraphicsQueue::~MetalGraphicsQueue() = default;
	
	std::unique_ptr<GhaGraphicsCommandBuffer> MetalGraphicsQueue::allocateCommandBuffer() {
		return std::make_unique<MetalGraphicsCommandBuffer>(allowBufferReuse);
	}
	
	void MetalGraphicsQueue::freeCommandBuffer(GhaGraphicsCommandBuffer &buffer) {
		//no op
	}

	void MetalGraphicsQueue::submit(std::vector<GraphicsSubmitInfo> const &submissions, GhaFence *signalFence) {
		@autoreleasepool {
			for(auto const &submission : submissions) {
				for(auto const &commandBuffer : submission.commandBuffers) {
					auto *metalCommandBuffer{ polyCast<MetalGraphicsCommandBuffer>(commandBuffer.get()) };
					if(metalCommandBuffer->getCommandBufferUsage() == CommandBufferUsage::OneTimeSubmit && metalCommandBuffer->bufferHasBeenUsed()){
						CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Error, "GraphicsCommandBuffer recorded with CommandBufferUsage::OneTimeSubmit has already been used. Only buffers recorded with CommandBufferUsage::Default can submitted multiples times after being recorded once.");
						break;
					}
					
					id<MTLCommandBuffer> executionBuffer{ [commandQueue commandBuffer] };
					
					for(auto &pass : metalCommandBuffer->getEncodedRenderPasses()) {
						id<MTLRenderCommandEncoder> encoder{ pass.begin(executionBuffer) };
						
						//Inject the wait semaphore into each buffer
						for (auto const &semaphore : submission.waitSemaphores) {
							auto *metalSemaphore{ polyCast<MetalSemaphore const>(semaphore.first.get()) };
							MTLRenderStages const waitStage{ convertStage(semaphore.second) };
							
							[encoder waitForFence:metalSemaphore->getFence()
									 beforeStages:waitStage];
						}
						
						//Excute all recorded commands for the encoder
						for(auto &command : pass.commands) {
							command(encoder);
						}
						
						[encoder endEncoding];
					}
					
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
