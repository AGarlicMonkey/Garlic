#pragma once

#include <Clove/Delegate/MultiCastDelegate.hpp>
#include <Clove/Expected.hpp>
#include <Clove/Graphics/GhaImage.hpp>
#include <Clove/Graphics/GhaImageView.hpp>

namespace garlic::clove {
    class GhaSemaphore;
    class FrameBuffer;
    struct GraphicsSubmitInfo;
}

namespace garlic::clove {
    /**
     * @brief Contains an array of 1-N images that can be rendered to.
     */
    class RenderTarget {
        //VARIABLES
    public:
        /**
         * @brief Gets called when properties of this render target are about to change. 
         * Allowing objects referencing any (i.e ImageViews) to be reset ahead of time.
         */
        MultiCastDelegate<void()> onPropertiesChangedBegin{};
        /**
         * @brief Gets called once properties of the render target have finished changing.
         * Allowing objects referencing those properties to be created
         */
        MultiCastDelegate<void()> onPropertiesChangedEnd{};

        //FUNCTIONS
    public:
        RenderTarget() = default;

        RenderTarget(RenderTarget const &other)     = delete;
        RenderTarget(RenderTarget &&other) noexcept = default;

        RenderTarget &operator=(RenderTarget const &other) = delete;
        RenderTarget &operator=(RenderTarget &&other) noexcept = default;

        virtual ~RenderTarget() = default;

        /**
         * @brief Aquire the next available image that can be rendered to.
         * @param frameId Which frame is currently being rendered. This 
         * number will be from 0 - N-1 where N is the total number of frames in flight.
         * @return Returns the image index for the getImageViews array.
         */
        virtual Expected<uint32_t, std::string> aquireNextImage(size_t const frameId) = 0;

        /**
         * @brief Submit graphics command buffers to be presented on this RenderTarget.
         * @param imageIndex The image index of the getImageViews array this submission is for.
         * @param frameId Which frame is currently being rendered. This 
         * number will be from 0 - N-1 where N is the total number of frames in flight.
         * @param submission The graphics queue submission that uses the imageIndex image.
         */
        virtual void submit(uint32_t imageIndex, size_t const frameId, GraphicsSubmitInfo submission) = 0;

        virtual GhaImage::Format getImageFormat() const = 0;
        virtual vec2ui getSize() const                  = 0;

        virtual std::vector<std::shared_ptr<GhaImageView>> getImageViews() const = 0;
    };
}