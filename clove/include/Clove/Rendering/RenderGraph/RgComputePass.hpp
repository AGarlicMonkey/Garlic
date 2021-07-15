#pragma once

#include "Clove/Rendering/RenderGraph/RgPass.hpp"
#include "Clove/Rendering/RenderGraph/Bindings.hpp"

#include <vector>

namespace garlic::clove {
    class RgComputePass : public RgPass {
        //TYPES
    public:
        struct Descriptor {

        };

        /**
         * @brief Represents a single unit of work. A single dispatch
         */
        struct Submission {

        };

        //VARIABLES
    private:
        Descriptor descriptor{};
        std::vector<Submission> submissions{};

        //FUNCTIONS
    public:
        RgComputePass() = delete;
        RgComputePass(RgPassIdType id);

        RgComputePass(RgComputePass const &other) = delete;
        RgComputePass(RgComputePass &&other) noexcept;

        RgComputePass &operator=(RgComputePass const &other) = delete;
        RgComputePass &operator=(RgComputePass &&other) noexcept;

        ~RgComputePass();

        std::unordered_set<RgResourceIdType> getInputResources() const override;
        std::unordered_set<RgResourceIdType> getOutputResources() const override;

        inline void addSubmission(Submission submission);

        inline Descriptor const &getDescriptor() const;
        inline std::vector<Submission> const &getSubmissions() const;
    };
}

#include "RgComputePass.inl"