#pragma once

namespace clv::gfx{
	enum class API{
		None,
		OpenGL4,
	#if CLV_PLATFORM_WINDOWS
		DirectX11
	#endif
	};

	//TODO: These need adjusting. Also probably need a different way of doing this because the SBO uses this
	enum class ShaderType{
		Vertex,
		Pixel,
		Vertex2D,
		Pixel2D
	};
}