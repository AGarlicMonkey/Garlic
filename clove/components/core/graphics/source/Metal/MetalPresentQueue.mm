#include "Clove/Graphics/Metal/MetalPresentQueue.hpp"

#include "Clove/Graphics/Metal/MetalView.hpp"
#include "Clove/Graphics/Metal/MetalSwapchain.hpp"
#include "Clove/Graphics/Metal/MetalImageView.hpp"

#include <Clove/Cast.hpp>

namespace garlic::clove {
	MetalPresentQueue::MetalPresentQueue(id<MTLCommandQueue> commandQueue, MetalView *view)
		: commandQueue{ [commandQueue retain] }
		, view{ [view retain] } {
	}
	
	MetalPresentQueue::MetalPresentQueue(MetalPresentQueue &&other) noexcept = default;
	
	MetalPresentQueue &MetalPresentQueue::operator=(MetalPresentQueue &&other) noexcept = default;
	
	MetalPresentQueue::~MetalPresentQueue() {
		[commandQueue release];
		[view release];
	}
	
	Result MetalPresentQueue::present(PresentInfo const &presentInfo) {
		@autoreleasepool{
			MetalSwapchain *swapchain{ polyCast<MetalSwapchain>(presentInfo.swapChain.get()) };
			id<MTLTexture> texture{ polyCast<MetalImageView>(swapchain->getImageViews()[presentInfo.imageIndex].get())->getTexture() };
			id<CAMetalDrawable> drawable{ view.metalLayer.nextDrawable };
		
			id<MTLCommandBuffer> commandBuffer{ [commandQueue commandBuffer] };
		
			id<MTLBlitCommandEncoder> encoder{ [commandBuffer blitCommandEncoder] };
			[encoder copyFromTexture:texture toTexture:drawable.texture];
			[encoder endEncoding];
		
			[commandBuffer presentDrawable:drawable];
			[commandBuffer commit];

			swapchain->markIndexAsFree(presentInfo.imageIndex);
		}
	}
}
