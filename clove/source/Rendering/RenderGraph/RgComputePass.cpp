#include "Clove/Rendering/RenderGraph/RgComputePass.hpp"

namespace garlic::clove {
    RgComputePass::RgComputePass(RgPassIdType id)
        : RgPass{ id } {
    }

    RgComputePass::RgComputePass(RgComputePass &&other) noexcept = default;

    RgComputePass &RgComputePass::operator=(RgComputePass &&other) noexcept = default;

    RgComputePass::~RgComputePass() = default;

    std::unordered_set<RgResourceIdType> RgComputePass::getInputResources() const {
        return {};
    }

    std::unordered_set<RgResourceIdType> RgComputePass::getOutputResources() const {
        return {};
    }
}