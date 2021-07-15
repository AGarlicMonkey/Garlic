#pragma once

#include "Clove/Rendering/RenderGraph/RgPass.hpp"

#include <vector>

namespace garlic::clove {
    class RgTransferPass : public RgPass {
        //TYPES
    public:
        struct BufferWrite {
            RgResourceIdType bufferId{};
            std::vector<std::byte> data;
            size_t offset{};
            size_t size;
        };

        //VARIABLES
    private:
        BufferWrite writePass{};

        //FUNCTIONS
    public:
        RgTransferPass() = delete;
        RgTransferPass(RgPassIdType id, BufferWrite writePass);

        RgTransferPass(RgTransferPass const &other) = delete;
        RgTransferPass(RgTransferPass &&other) noexcept;

        RgTransferPass &operator=(RgTransferPass const &other) = delete;
        RgTransferPass &operator=(RgTransferPass &&other) noexcept;

        ~RgTransferPass();

        std::unordered_set<RgResourceIdType> getInputResources() const override;
        std::unordered_set<RgResourceIdType> getOutputResources() const override;

        inline BufferWrite const &getWriteOperation() const;
    };
}

#include "RgTransferPass.inl"