#include "Clove/UI/Image.hpp"

#include "Clove/Application.hpp"
#include "Clove/Rendering/ForwardRenderer3D.hpp"
#include "Clove/Rendering/RenderingHelpers.hpp"

#include <Clove/Graphics/GhaDevice.hpp>
#include <Clove/Graphics/GhaFactory.hpp>
#include <Clove/Graphics/GhaImage.hpp>
#include <Clove/Graphics/GhaImageView.hpp>
#include <Clove/Maths/Maths.hpp>
#include <Clove/Maths/MathsHelpers.hpp>

namespace garlic::clove {
    Image::Image() {
        vec2ui constexpr imageDimensions{ 1, 1 };
        uint32_t constexpr bytesPerTexel{ 4 };
        uint32_t constexpr white{ 0xffffffff };

        GhaFactory &factory{ *Application::get().getGraphicsDevice()->getGraphicsFactory() };

        GhaImage::Descriptor constexpr imageDescriptor{
            .type        = GhaImage::Type::_2D,
            .usageFlags  = GhaImage::UsageMode::TransferDestination | GhaImage::UsageMode::Sampled,
            .dimensions  = imageDimensions,
            .format      = GhaImage::Format::R8G8B8A8_SRGB,
            .sharingMode = SharingMode::Exclusive,
        };

        image     = createImageWithData(factory, imageDescriptor, &white, bytesPerTexel);
        imageView = *factory.createImageView(*image, GhaImageView::Descriptor{
                                                         .type       = GhaImageView::Type::_2D,
                                                         .layer      = 0,
                                                         .layerCount = 1,
                                                     });
    }

    Image::Image(std::shared_ptr<GhaImage> graphicsImage)
        : image(std::move(graphicsImage)) {
        GhaFactory &factory{ *Application::get().getGraphicsDevice()->getGraphicsFactory() };
        imageView = *factory.createImageView(*image, GhaImageView::Descriptor{
                                                         .type       = GhaImageView::Type::_2D,
                                                         .layer      = 0,
                                                         .layerCount = 1,
                                                     });
    }

    Image::Image(Image const &other) = default;

    Image::Image(Image &&other) noexcept = default;

    Image &Image::operator=(Image const &other) = default;

    Image &Image::operator=(Image &&other) noexcept = default;

    Image::~Image() = default;

    void Image::draw(vec2f const &drawSpace) {
        vec2f const screenHalfSize{ drawSpace / 2.0f };

        //Move the position to origin at the top left
        vec2f const pos{ position.x - screenHalfSize.x, -position.y + screenHalfSize.y };

        mat4f const model{ translate(mat4f{ 1.0f }, { pos, 0.0f }) * rotate(mat4f{ 1.0f }, this->rotation, { 0.0f, 0.0f, 1.0f }) * scale(mat4f{ 1.0f }, vec3f{ size, 0.0f }) };
        mat4f const projection{ createOrthographicMatrix(-screenHalfSize.x, screenHalfSize.x, -screenHalfSize.y, screenHalfSize.y) };

        Application::get().getRenderer()->submitWidget(imageView, projection * model);
    }
}