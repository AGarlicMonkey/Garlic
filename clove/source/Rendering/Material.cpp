#include "Clove/Rendering/Material.hpp"

#include "Clove/Application.hpp"
#include "Clove/Rendering/RenderingHelpers.hpp"

#include <Clove/Graphics/GhaDevice.hpp>
#include <Clove/Graphics/GhaFactory.hpp>
#include <Clove/Graphics/GhaImage.hpp>
#include <Clove/Graphics/GhaImageView.hpp>

namespace garlic::clove {
    std::weak_ptr<garlic::clove::GhaImage> Material::defaultImage{};

    Material::Material() {
        using namespace garlic::clove;

        GhaFactory &factory{ *Application::get().getGraphicsDevice()->getGraphicsFactory() };

        if(defaultImage.use_count() == 0) {
            vec2f constexpr imageDimensions{ 1.0f, 1.0f };
            uint32_t constexpr bytesPerTexel{ 4 };
            uint32_t constexpr white{ 0xffffffff };

            GhaImage::Descriptor const imageDescriptor{
                .type        = GhaImage::Type::_2D,
                .usageFlags  = GhaImage::UsageMode::TransferDestination | GhaImage::UsageMode::Sampled,
                .dimensions  = imageDimensions,
                .format      = GhaImage::Format::R8G8B8A8_SRGB,
                .sharingMode = SharingMode::Exclusive,
            };

            std::shared_ptr<GhaImage> image = createImageWithData(factory, imageDescriptor, &white, bytesPerTexel);

            defaultImage = image;

            diffuseImage  = image;
            specularImage = image;
        } else {
            diffuseImage  = defaultImage.lock();
            specularImage = defaultImage.lock();
        }

        GhaImageView::Descriptor const viewDescriptor{
            .type       = GhaImageView::Type::_2D,
            .layer      = 0,
            .layerCount = 1,
        };

        diffuseView  = *factory.createImageView(*diffuseImage, viewDescriptor);
        specularView = *factory.createImageView(*specularImage, viewDescriptor);
    }

    Material::Material(Material const &other) = default;

    Material::Material(Material &&other) noexcept = default;

    Material &Material::operator=(Material const &other) = default;

    Material &Material::operator=(Material &&other) noexcept = default;

    Material::~Material() = default;

    void Material::setDiffuseTexture(std::shared_ptr<GhaImage> image) {
        GhaFactory &factory{ *Application::get().getGraphicsDevice()->getGraphicsFactory() };

        diffuseImage = std::move(image);
        diffuseView  = *factory.createImageView(*diffuseImage, GhaImageView::Descriptor{
                                                                  .type       = GhaImageView::Type::_2D,
                                                                  .layer      = 0,
                                                                  .layerCount = 1,
                                                              });
    }

    void Material::setSpecularTexture(std::shared_ptr<GhaImage> image) {
        GhaFactory &factory{ *Application::get().getGraphicsDevice()->getGraphicsFactory() };

        specularImage = std::move(image);
        specularView  = *factory.createImageView(*specularImage, GhaImageView::Descriptor{
                                                                    .type       = GhaImageView::Type::_2D,
                                                                    .layer      = 0,
                                                                    .layerCount = 1,
                                                                });
    }
}