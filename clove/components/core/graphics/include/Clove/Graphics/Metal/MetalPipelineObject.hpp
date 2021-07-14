#pragma once

#include "Clove/Graphics/PipelineObject.hpp"

#include <MetalKit/MetalKit.h>

namespace garlic::clove {
	MTLRenderStages convertStage(PipelineStage stage);
}
