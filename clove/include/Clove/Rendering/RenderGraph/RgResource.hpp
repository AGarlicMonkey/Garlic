#pragma once

#include "Clove/Rendering/RenderGraph/RgId.hpp"

#include <unordered_set>

namespace garlic::clove {
    /**
     * @brief Base class for all resources in the graph. Keeps track of used passes
     */
    class RgResource {
        //VARIABLES
    private:
        RgResourceIdType id{ INVALID_RESOURCE_ID };

        std::unordered_set<RgPassIdType> writePasses{};
        std::unordered_set<RgPassIdType> readPasses{};

        //FUNCTIONS
    public:
        RgResource() = delete;
        RgResource(RgResourceIdType id);

        RgResource(RgResource const &other) = delete;
        RgResource(RgResource &&other) noexcept;

        RgResource &operator=(RgResource const &other) = delete;
        RgResource &operator=(RgResource &&other) noexcept;

        ~RgResource();

        inline void addWritePass(RgPassIdType pass);
        inline void addReadPass(RgPassIdType pass);

        inline RgResourceIdType getId() const;

        inline std::unordered_set<RgPassIdType> const &getWritePasses() const;
        inline std::unordered_set<RgPassIdType> const &getReadPasses() const;
    };
}

#include "RgResource.inl"