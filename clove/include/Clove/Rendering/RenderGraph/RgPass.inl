namespace garlic::clove {
    void RgPass::addInputResource(ResourceIdType resource) {
        inputResources.emplace(resource);
    }

    void RgPass::addOutputResource(ResourceIdType resource) {
        outputResources.emplace(resource);
    }

    PassIdType RgPass::getId() const {
        return id;
    }

    std::unordered_set<ResourceIdType> const &RgPass::getInputResource() const {
        return inputResources;
    }

    std::unordered_set<ResourceIdType> const &RgPass::getOutputResource() const {
        return outputResources;
    }
}