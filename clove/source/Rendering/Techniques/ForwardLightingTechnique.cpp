#include "Clove/Rendering/Techniques/ForwardLightingTechnique.hpp"

#include "Clove/Rendering/RenderPasses/ForwardColourPass.hpp"
#include "Clove/Rendering/RenderPasses/DirectionalLightPass.hpp"
#include "Clove/Rendering/RenderPasses/PointLightPass.hpp"

namespace garlic::clove {
    Technique createForwardLightingTechnique() {
        return Technique{
            .passIds = std::set<GeometryPass::Id>{
                GeometryPass::getId<ForwardColourPass>(),
                GeometryPass::getId<DirectionalLightPass>(),
                GeometryPass::getId<PointLightPass>(),
            },
        };
    }
}