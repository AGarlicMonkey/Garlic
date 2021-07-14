#include "Clove/Graphics/Metal/MetalPipelineObject.hpp"

#include <Clove/Log/Log.hpp>

namespace garlic::clove {
	MTLRenderStages convertStage(PipelineStage stage) {
		//Metal only has 2 stages: Vertex and Fragment
		switch (stage) {
			case PipelineStage::VertexInput:
			case PipelineStage::VertexShader:
				return MTLRenderStageVertex;
			case PipelineStage::PixelShader:
			case PipelineStage::EarlyPixelTest:
			case PipelineStage::LatePixelTest:
			case PipelineStage::ColourAttachmentOutput:
				return MTLRenderStageFragment;
			default:
				CLOVE_ASSERT(false, "{0}: Provided stage does not convert properly", CLOVE_FUNCTION_NAME_PRETTY);
				return MTLRenderStageFragment;
		}
	}
}
