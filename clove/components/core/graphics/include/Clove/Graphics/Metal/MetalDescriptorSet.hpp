#pragma once

#include "Clove/Graphics/GhaDescriptorSet.hpp"

#include "Clove/Graphics/GhaShader.hpp"

#include <unordered_map>
#include <MetalKit/MetalKit.h>

namespace garlic::clove {
	class GhaDescriptorSetLayout;
}

namespace garlic::clove {
	class MetalDescriptorSet : public GhaDescriptorSet {
		//VARIABLES
	private:
		id<MTLArgumentEncoder> vertexEncoder{ nullptr };
		id<MTLArgumentEncoder> pixelEncoder{ nullptr };
		
		std::shared_ptr<GhaDescriptorSetLayout> layout{ nullptr };
		
		//FUNCTIONS
	public:
		MetalDescriptorSet() = delete;
		MetalDescriptorSet(id<MTLArgumentEncoder> vertexEncoder, id<MTLArgumentEncoder> pixelEncoder, std::shared_ptr<GhaDescriptorSetLayout> layout);
		
		MetalDescriptorSet(MetalDescriptorSet const &other) = delete;
		MetalDescriptorSet(MetalDescriptorSet &&other) noexcept;
		
		MetalDescriptorSet &operator=(MetalDescriptorSet const &other) = delete;
		MetalDescriptorSet &operator=(MetalDescriptorSet &&other) noexcept;
		
		~MetalDescriptorSet();
		
		void map(GhaBuffer const &buffer, size_t const offset, size_t const range, DescriptorType const descriptorType, uint32_t const bindingSlot) override;
		
		void map(GhaImageView const &imageView, GhaSampler const &sampler, GhaImage::Layout const layout, uint32_t const bindingSlot) override;
		void map(std::span<std::shared_ptr<GhaImageView>> imageViews, GhaSampler const &sampler, GhaImage::Layout const layout, uint32_t const bindingSlot) override;
		
		inline id<MTLArgumentEncoder> getVertexEncoder() const;
		inline id<MTLArgumentEncoder> getPixelEncoder() const;
		
	private:
		GhaShader::Stage getStageFromBindingSlot(uint32_t const bindingSlot);
	};
}

#include "MetalDescriptorSet.inl"
