#include "Clove/Graphics/Metal/MetalGraphicsQueue.hpp"

#include "Clove/Graphics/Metal/MetalGraphicsCommandBuffer.hpp"
#include "Clove/Graphics/Metal/MetalView.hpp"

#include <Clove/Cast.hpp>

namespace garlic::clove {
	MetalGraphicsQueue::MetalGraphicsQueue(id<MTLCommandQueue> commandQueue)
		: commandQueue{ [commandQueue retain] } {
	}
	
	MetalGraphicsQueue::MetalGraphicsQueue(MetalGraphicsQueue &&other) noexcept = default;
	
	MetalGraphicsQueue& MetalGraphicsQueue::operator=(MetalGraphicsQueue &&other) noexcept = default;
	
	MetalGraphicsQueue::~MetalGraphicsQueue() = default;
	
	std::unique_ptr<GhaGraphicsCommandBuffer> MetalGraphicsQueue::allocateCommandBuffer() {
		return std::make_unique<MetalGraphicsCommandBuffer>();
	}
	
	void MetalGraphicsQueue::freeCommandBuffer(GhaGraphicsCommandBuffer &buffer) {
		//no op
	}

	void MetalGraphicsQueue::submit(std::vector<GraphicsSubmitInfo> const &submissions, GhaFence const *signalFence) {
		@autoreleasepool {
			for(auto const &submission : submissions) {
				//TODO: Fences / semahpores
				//[currentEncoder updateFence:nullptr afterStages:MTLRenderStageFragment];
			
				for(auto const &commandBuffer : submission.commandBuffers) {
					id<MTLCommandBuffer> mtlCommandBuffer{ [commandQueue commandBuffer] };
				
					polyCast<MetalGraphicsCommandBuffer>(commandBuffer.get())->executeCommands(mtlCommandBuffer);
					//[mtlCommandBuffer release];
				}
			}
		}
	}
}
