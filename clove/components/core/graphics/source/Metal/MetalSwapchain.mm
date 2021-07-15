#include "Clove/Graphics/Metal/MetalSwapchain.hpp"

#include "Clove/Graphics/Metal/MetalImage.hpp"
#include "Clove/Graphics/Metal/MetalImageView.hpp"

#include <Clove/Log/Log.hpp>

namespace garlic::clove {
	MetalSwapchain::MetalSwapchain(std::vector<std::shared_ptr<GhaImage>> images, std::vector<std::shared_ptr<GhaImageView>> imageViews, GhaImage::Format imageFormat, vec2ui imageSize)
		: images{ std::move(images) }
		, imageViews{ std::move(imageViews) }
		, imageFormat{ imageFormat }
		, imageSize{ imageSize } {
	}
	
	MetalSwapchain::MetalSwapchain(MetalSwapchain &&other) noexcept = default;
	
	MetalSwapchain& MetalSwapchain::operator=(MetalSwapchain &&other) noexcept = default;
	
	MetalSwapchain::~MetalSwapchain() = default;
	
	std::pair<uint32_t, Result> MetalSwapchain::aquireNextImage(GhaSemaphore const *availableSemaphore) {
		//TODO: will signal semaphore immediately semaphores
		//TODO: Handle resizing;
		
		if(imageQueue.empty()) {
			CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Error, "{0} has no available images", CLOVE_FUNCTION_NAME_PRETTY);
			return { -1, Result::Unkown };
		}
		
		uint32_t const availableIndex{ imageQueue.front() };
		imageQueue.pop();
		
		return { availableIndex, Result::Success };
	}

	GhaImage::Format MetalSwapchain::getImageFormat() const {
		return imageFormat;
	}
	
	vec2ui MetalSwapchain::getSize() const {
		return imageSize;
	}

	std::vector<std::shared_ptr<GhaImageView>> MetalSwapchain::getImageViews() const {
		return imageViews;
	}
	
	void MetalSwapchain::markIndexAsFree(uint32_t index) {
		imageQueue.push(index);
	}
}
