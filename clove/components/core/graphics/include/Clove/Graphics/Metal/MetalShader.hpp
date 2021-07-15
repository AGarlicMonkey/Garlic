#pragma once

#include "Clove/Graphics/GhaShader.hpp"

#include <MetalKit/MetalKit.h>

namespace garlic::clove {
	class MetalShader : public GhaShader {
		//VARIABLES
	private:
		id<MTLFunction> function;
		
		//FUNCTIONS
	public:
		MetalShader() = delete;
		MetalShader(id<MTLFunction> function);
		
		MetalShader(MetalShader const &other) = delete;
		MetalShader(MetalShader &&other) noexcept;
		
		MetalShader& operator=(MetalShader const &other) = delete;
		MetalShader& operator=(MetalShader &&other) noexcept;
		
		~MetalShader();
		
		id<MTLFunction> getFunction() const;
	};
}
