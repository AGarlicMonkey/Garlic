#pragma once

#include <Clove/Graphics/GhaDescriptorSetLayout.hpp>
#include <Clove/Graphics/GhaGraphicsPipelineObject.hpp>
#include <Clove/Graphics/GhaRenderPass.hpp>
#include <Clove/Graphics/GhaSampler.hpp>
#include <Clove/Graphics/GhaShader.hpp>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace garlic::clove {
    class GhaFactory;
}

namespace garlic::clove {
    /**
     * @brief Stores objects that will be used regardless of what frame is being rendered (shaders, pipelines, etc.)
     */
    class RgGlobalCache {
        //TYPES
    private:
        using PoolId = uint64_t;

        //VARIABLES
    private:
        std::shared_ptr<GhaFactory> factory{ nullptr };

        std::unordered_map<std::string, std::shared_ptr<GhaShader>> shaders{};
        std::unordered_map<PoolId, std::shared_ptr<GhaSampler>> samplers{};
        std::unordered_map<PoolId, std::shared_ptr<GhaRenderPass>> renderPasses{};
        std::unordered_map<PoolId, std::shared_ptr<GhaDescriptorSetLayout>> descriptorSetLayouts{};
        std::unordered_map<PoolId, std::shared_ptr<GhaGraphicsPipelineObject>> graphicsPipelines{};

        //FUNCTIONS
    public:
        RgGlobalCache() = delete;
        RgGlobalCache(std::shared_ptr<GhaFactory> ghaFactory);

        RgGlobalCache(RgGlobalCache const &other) = delete;
        RgGlobalCache(RgGlobalCache &&other) noexcept;

        RgGlobalCache &operator=(RgGlobalCache const &other) = delete;
        RgGlobalCache &operator=(RgGlobalCache &&other) noexcept;

        ~RgGlobalCache();

        std::shared_ptr<GhaShader> createShader(std::filesystem::path const &file, GhaShader::Stage shaderStage);
        std::shared_ptr<GhaShader> createShader(std::string_view source, std::unordered_map<std::string, std::string> includeSources, std::string_view shaderName, GhaShader::Stage shaderStage);

        std::shared_ptr<GhaSampler> createSampler(GhaSampler::Descriptor descriptor);

        std::shared_ptr<GhaRenderPass> createRenderPass(GhaRenderPass::Descriptor descriptor);
        std::shared_ptr<GhaDescriptorSetLayout> createDescriptorSetLayout(GhaDescriptorSetLayout::Descriptor descriptor);
        std::shared_ptr<GhaGraphicsPipelineObject> createGraphicsPipelineObject(GhaGraphicsPipelineObject::Descriptor descriptor);
    };
}