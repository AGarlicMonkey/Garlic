namespace garlic::clove {
    void RgComputePass::addSubmission(Submission submission) {

    }

    RgComputePass::Descriptor const &RgComputePass::getDescriptor() const {
        return descriptor;
    }

    std::vector<RgComputePass::Submission> const &RgComputePass::getSubmissions() const {
        return submissions;
    }
}