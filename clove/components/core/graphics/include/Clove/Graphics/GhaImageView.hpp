#pragma once

#include "Clove/Graphics/GhaImage.hpp"

#include <Clove/Maths/Vector.hpp>
#include <cinttypes>

namespace garlic::clove {
    /**
     * @brief Represents a view on a subset of a GhaImage.
     * @details GhaImages are not directly accessed by shaders for reading / writing.
     * Instead, a view onto that image is used.
     */
    class GhaImageView {
        //TYPES
    public:
        enum class Type {
            _2D,
            _3D,
            Cube
        };

        struct Descriptor {
            Type type;
            uint32_t layer{ 0 };      /**< If the image is an array or cube map, which element to start on (0 indexed). */
            uint32_t layerCount{ 1 }; /**< If the image is an array or cube map, how many elements to view. */
        };

        //FUNCTIONS
    public:
        virtual ~GhaImageView() = default;

        /**
         * @brief Returns the format of the viewed image.
         * @return 
         */
        virtual GhaImage::Format getImageFormat() const = 0;

        /**
         * @brief Returns the dimensions of the viewed image.
         * @return 
         */
        virtual vec2ui const &getImageDimensions() const = 0;
    };
}
