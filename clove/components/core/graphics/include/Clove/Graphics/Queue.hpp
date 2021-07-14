#pragma once

#include <Clove/Definitions.hpp>

namespace garlic::clove {
    using QueueFlagsType = uint8_t;
    enum class QueueFlags : QueueFlagsType {
        None         = 0,
        Transient    = 1 << 0, /**< Specifies that command buffers will be freed or reset shortly after using them. */
        ReuseBuffers = 1 << 1  /**< Allows buffers to be reset and recorded to multiple times. Normally a buffer needs to be freed and realloacted once used. */
    };

    struct CommandQueueDescriptor {
        QueueFlags flags; /**< Flags specifying how buffers allocated from a queue will be used. */
    };

    CLOVE_ENUM_BIT_FLAG_OPERATORS(QueueFlags, QueueFlagsType)
}
