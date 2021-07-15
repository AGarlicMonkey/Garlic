namespace garlic::clove {
    void RgResource::addWritePass(RgPassIdType pass) {
        writePasses.emplace(pass);
    }

    void RgResource::addReadPass(RgPassIdType pass) {
        readPasses.emplace(pass);
    }

    RgResourceIdType RgResource::getId() const {
        return id;
    }

    std::unordered_set<RgPassIdType> const &RgResource::getWritePasses() const {
        return writePasses;
    }

    std::unordered_set<RgPassIdType> const &RgResource::getReadPasses() const {
        return readPasses;
    }
}