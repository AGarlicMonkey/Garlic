#pragma once

#include <cinttypes>
#include <cstddef>

namespace garlic::clove{
    using RgResourceIdType = size_t;
    using RgPassIdType     = size_t;

    static RgResourceIdType constexpr INVALID_RESOURCE_ID{ 0 };
    static RgPassIdType constexpr INVALID_PASS_ID{ 0 };
}
