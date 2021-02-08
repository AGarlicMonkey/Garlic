#pragma once

#include "Clove/Audio/AhaDevice.hpp"

#include <AL/alc.h>

namespace garlic::clove {
    class OpenAlFactory;
}

namespace garlic::clove {
    /**
     * @brief Represents the actual hardware used for audio rendering. Can be quieried for info about the device.
     */
    class OpenAlDevice : public AhaDevice {
        //VARIABLES
    private:
        ALCdevice *alDevice{ nullptr };
        ALCcontext *alContext{ nullptr };

        std::shared_ptr<OpenAlFactory> factory;

        //FUNCTIONS
    public:
        OpenAlDevice();

        OpenAlDevice(OpenAlDevice const &other) = delete;
        OpenAlDevice(OpenAlDevice &&other) noexcept;

        OpenAlDevice &operator=(OpenAlDevice const &other) = delete;
        OpenAlDevice &operator=(OpenAlDevice &&other) noexcept;

        ~OpenAlDevice();

        std::shared_ptr<AhaFactory> getAudioFactory() const override;
    };
}