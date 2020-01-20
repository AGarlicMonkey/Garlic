#pragma once

#include "Clove/Graphics/Core/GraphicsTypes.hpp"

namespace clv::gfx{
	enum class ShaderOutputType{
		GLSL,
		MSL
	};
}

namespace clv::gfx::ShaderCompiler{
	//TODO: Rename to transpile
	std::string compile(const std::string& source, ShaderType type, ShaderOutputType outputType);
}