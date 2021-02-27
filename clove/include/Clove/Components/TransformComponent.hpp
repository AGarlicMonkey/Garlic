#pragma once

#include <Clove/Maths/MathsTypes.hpp>

namespace garlic::clove {
    /**
     * @brief A TransformComponent holds the position, rotation and scale of an Entity.
     */
    struct TransformComponent {
        vec3f position{ 0.0f, 0.0f, 0.0f };
        quatf rotation{ vec3f{ 0.0f, 0.0f, 0.0f } };
        vec3f scale{ 1.0f, 1.0f, 1.0f };

        /**
         * @brief This matrix is calculated inside TransformLayer. One frame needs to pass
         * before it is updated
         */
        mat4f worldMatrix{ 1.0f };

        vec3f getForward() const;
        vec3f getRight() const;
        vec3f getUp() const;
    };
}