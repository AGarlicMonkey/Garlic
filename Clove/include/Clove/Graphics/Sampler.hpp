#pragma once

namespace clv::gfx {
    /**
     * @brief Describes how a shader should sample a GraphicsImage.
     */
    class Sampler {
        //TYPES
    public:
        enum class Filter {
            Nearest,
            Linear
        };
        enum class AddressMode {
            Repeat,
            MirroredRepeat,
            ClampToEdge,
            ClampToBorder
        };

        struct Descriptor {
            Filter minFilter;
            Filter magFilter;
            AddressMode addressModeU;
            AddressMode addressModeV;
            AddressMode addressModeW;
            bool enableAnisotropy{ false };
            float maxAnisotropy{ 1.0f };
        };

        //FUNCTIONS
    public:
        virtual ~Sampler() = default;
    };
}