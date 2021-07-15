namespace garlic::clove {
    void RgRenderPass::addSubmission(Submission submission) {
        submissions.emplace_back(std::move(submission));
    }

    RgRenderPass::Descriptor const &RgRenderPass::getDescriptor() const {
        return descriptor;
    }

    std::vector<RgRenderPass::Submission> const &RgRenderPass::getSubmissions() const {
        return submissions;
    }
}