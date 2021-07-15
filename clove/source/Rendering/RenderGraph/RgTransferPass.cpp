#include "Clove/Rendering/RenderGraph/RgTransferPass.hpp"

namespace garlic::clove {
    RgTransferPass::RgTransferPass(RgPassIdType id, BufferWrite writePass)
        : RgPass{ id }
        , writePass{ std::move(writePass) } {
    }

    RgTransferPass::RgTransferPass(RgTransferPass &&other) noexcept = default;

    RgTransferPass &RgTransferPass::operator=(RgTransferPass &&other) noexcept = default;

    RgTransferPass::~RgTransferPass() = default;

    std::unordered_set<RgResourceIdType> RgTransferPass::getInputResources() const {
        return {};
    }

    std::unordered_set<RgResourceIdType> RgTransferPass::getOutputResources() const {
        return { writePass.bufferId };
    }
}