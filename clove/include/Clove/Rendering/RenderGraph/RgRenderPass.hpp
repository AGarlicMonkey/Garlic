#pragma once

#include "Clove/Rendering/RenderGraph/Bindings.hpp"
#include "Clove/Rendering/RenderGraph/RgPass.hpp"
#include "Clove/Rendering/RenderGraph/RgShader.hpp"

#include <vector>

namespace {
    class RgGlobalCache;
}

namespace garlic::clove {
    /**
     * @brief Represents an entire render pass in the graph.
     */
    class RgRenderPass : public RgPass {
        //TYPES
    public:
        struct Descriptor {
            RgShader vertexShader{};
            RgShader pixelShader{};

            vec2i viewportPosition{ 0 };
            vec2ui viewportSize{ 0 };
            //TODO: raster state
            bool depthTest{ true };
            bool depthWrite{ true };
            bool enableBlending{ true };

            std::vector<RgRenderTargetBinding> renderTargets{};
            RgDepthStencilBinding depthStencil{};
        };

        /**
         * @brief Represents a unit of work, single draw indexed call.
         */
        struct Submission {
            RgResourceIdType vertexBuffer{};
            RgResourceIdType indexBuffer{};

            std::vector<RgBufferBinding> shaderUbos{};
            std::vector<RgImageBindng> shaderCombinedImageSamplers{};

            size_t indexCount{ 0 };
        };

        //VARIABLES
    private:
        Descriptor descriptor{};
        std::vector<Submission> submissions{};

        //FUNCTIONS
    public:
        RgRenderPass() = delete;
        RgRenderPass(RgPassIdType id, Descriptor descriptor);

        RgRenderPass(RgRenderPass const &other) = delete;
        RgRenderPass(RgRenderPass &&other) noexcept;

        RgRenderPass &operator=(RgRenderPass const &other) = delete;
        RgRenderPass &operator=(RgRenderPass &&other) noexcept;

        ~RgRenderPass();

        std::unordered_set<RgResourceIdType> getInputResources() const override;
        std::unordered_set<RgResourceIdType> getOutputResources() const override;

        inline void addSubmission(Submission submission);

        inline Descriptor const &getDescriptor() const;
        inline std::vector<Submission> const &getSubmissions() const;
    };
}

#include "RgRenderPass.inl"