#include "Clove/Rendering/RenderGraph/RgPass.hpp"

namespace garlic::clove {
    RgPass::RgPass(RgPassIdType id)
        : id{ id } {
    }

    RgPass::RgPass(RgPass &&other) noexcept = default;

    RgPass &RgPass::operator=(RgPass &&other) noexcept = default;

    RgPass::~RgPass() = default;
}