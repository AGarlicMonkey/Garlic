#pragma once

#include "Clove/Graphics/GhaSampler.hpp"

#include <MetalKit/MetalKit.h>

namespace garlic::clove {
	class MetalSampler : public GhaSampler {
		//VARIABLES
	private:
		id<MTLSamplerState> samplerState{ nullptr };
		
		//FUNCTIONS
	public:
		MetalSampler() = delete;
		MetalSampler(id<MTLSamplerState> samplerState);
		
		MetalSampler(MetalSampler const &other) = delete;
		MetalSampler(MetalSampler &&other) noexcept;
		
		MetalSampler& operator=(MetalSampler const &other) = delete;
		MetalSampler& operator=(MetalSampler &&other) noexcept;
		
		~MetalSampler();
		
		inline id<MTLSamplerState> getSamplerState() const;
	};
}

#include "MetalSampler.inl"
