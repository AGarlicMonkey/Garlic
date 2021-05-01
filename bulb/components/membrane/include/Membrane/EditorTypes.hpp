#pragma once

namespace garlic::membrane {
    // clang-format off
    public enum class ComponentType {
        Transform,
        Mesh,
        PointLight,
    };

    public value struct Vector3 {
        float x;
        float y;
        float z;

        Vector3(float x, float y, float z)
            : x{ x }
            , y{ y }
            , z{ z } {
        }
    };
    // clang-format on
}