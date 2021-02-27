#pragma once

#include <Clove/Maths/Matrix.hpp>
#include <Clove/Maths/Vector.hpp>

namespace garlic::clove {
    struct DirectionalLightComponent {
        /**
         * @brief The direction the light faces.
         */
        vec3f direction{ 0.0f, 0.0f, 0.0f };

        vec3f ambientColour{ 0.01f, 0.01f, 0.01f };
        vec3f diffuseColour{ 0.75f, 0.75f, 0.75f };
        vec3f specularColour{ 1.0f, 1.0f, 1.0f };
    };
}