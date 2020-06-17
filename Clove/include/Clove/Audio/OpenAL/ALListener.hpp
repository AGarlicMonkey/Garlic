#pragma once

#include "Clove/Audio/AudioListener.hpp"

#include <AL/al.h>

namespace clv {
    class ALListener : public AudioListener{
        //FUNCTIONS
    public:
        ALListener();

        ALListener(const ALListener& other) = delete;
        ALListener(ALListener&& other) noexcept;

        ALListener& operator=(const ALListener& other) = delete;
        ALListener& operator=(ALListener&& other) noexcept;

        ~ALListener();

        void setPosition(const clv::mth::vec3f& position) override;
        void setVelocity(const clv::mth::vec3f& velocity) override;
    };
}