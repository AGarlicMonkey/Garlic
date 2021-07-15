#include "Clove/Graphics/Metal/MetalShader.hpp"

namespace garlic::clove {
	MetalShader::MetalShader(id<MTLFunction> function)
		: function{ [function retain] }{
	}
	
	MetalShader::MetalShader(MetalShader &&other) noexcept = default;
	
	MetalShader& MetalShader::operator=(MetalShader &&other) noexcept = default;
	
	MetalShader::~MetalShader() {
		[function release];
	}
	
	id<MTLFunction> MetalShader::getFunction() const {
		return function;
	}
}
