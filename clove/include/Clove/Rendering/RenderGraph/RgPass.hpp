#pragma once

#include "Clove/Rendering/RenderGraph/RgId.hpp"

#include <unordered_set>

namespace garlic::clove {
    /**
     * @brief Represents an operation a series of resources.
     */
    class RgPass {
        //VARIABLES
    private:
        RgPassIdType id{ INVALID_PASS_ID };

        //FUNCTIONS
    public:
        RgPass() = delete;
        RgPass(RgPassIdType id);

        RgPass(RgPass const &other) = delete;
        RgPass(RgPass &&other) noexcept;

        RgPass &operator=(RgPass const &other) = delete;
        RgPass &operator=(RgPass &&other) noexcept;

        ~RgPass();

        inline RgPassIdType getId() const;

        virtual std::unordered_set<RgResourceIdType> getInputResources() const = 0;
        virtual std::unordered_set<RgResourceIdType> getOutputResources() const = 0;
    };
}

#include "RgPass.inl"