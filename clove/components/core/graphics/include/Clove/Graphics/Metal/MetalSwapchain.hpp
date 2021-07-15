#pragma once

#include "Clove/Graphics/GhaSwapchain.hpp"

#include <queue>

namespace garlic::clove {
	class GhaImage;
}

namespace garlic::clove {
	class MetalSwapchain : public GhaSwapchain {
		//VARIABLES
	private:
		std::vector<std::shared_ptr<GhaImage>> images{};
		std::vector<std::shared_ptr<GhaImageView>> imageViews{};
		
		GhaImage::Format imageFormat{};
		vec2ui imageSize{};
		
		std::queue<uint32_t> imageQueue{};
		
		//FUNCTIONS
	public:
		MetalSwapchain() = delete;
        MetalSwapchain(std::vector<std::shared_ptr<GhaImage>> images, std::vector<std::shared_ptr<GhaImageView>> imageViews, GhaImage::Format imageFormat, vec2ui imageSize);

        MetalSwapchain(MetalSwapchain const &other) = delete;
		MetalSwapchain(MetalSwapchain &&other) noexcept;
		
		MetalSwapchain& operator=(MetalSwapchain const &other) = delete;
		MetalSwapchain& operator=(MetalSwapchain &&other) noexcept;
		
		~MetalSwapchain();
		
		std::pair<uint32_t, Result> aquireNextImage(GhaSemaphore const *availableSemaphore) override;

		GhaImage::Format getImageFormat() const override;
		vec2ui getSize() const override;

		std::vector<std::shared_ptr<GhaImageView>> getImageViews() const override;
		
		/**
		 * @brief Tells the swapchain that the image index is free to use again.
		 */
		void markIndexAsFree(uint32_t index);
	};
}
