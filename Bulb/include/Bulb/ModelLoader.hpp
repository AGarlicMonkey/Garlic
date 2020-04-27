#pragma once

#include "Bulb/Rendering/Renderables/Model.hpp"

namespace clv::gfx{
	class GraphicsFactory;
}

namespace blb::ModelLoader {
	rnd::Model loadModel(std::string_view modelFilePath, const std::shared_ptr<clv::gfx::GraphicsFactory>& graphicsFactory);
}