#pragma once

#include "Bulb/ECS/System.hpp"

#include "Bulb/ECS/Components/AudioComponent.hpp"

//PortAudio type defas (see portaudio.h)
typedef void PaStream;
typedef unsigned long PaStreamCallbackFlags;

struct PaStreamCallbackTimeInfo;

//namespace blb::ecs::aud{
//	class AudioComponent;
//}

namespace blb::ecs {
	class AudioSystem : public System {
		//VARIABLES
	private:
		//FUNCTIONS
	public:
		AudioSystem();

		AudioSystem(const AudioSystem& other) = delete;
		AudioSystem(AudioSystem&& other) noexcept;

		AudioSystem& operator=(const AudioSystem& other) = delete;
		AudioSystem& operator=(AudioSystem&& other) noexcept;

		~AudioSystem();

		void update(World& world, clv::utl::DeltaTime deltaTime) override;

		void onComponentDestroyed(ComponentInterface* component) override;

	private:
		void startSound(AudioComponent* component, PlaybackMode playback);
		void pauseSound(AudioComponent* component);
		void stopSound(AudioComponent* component);

		static int soundPlayback_Loop(const void* inputBuffer, void* outputBuffer, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData);
		static int soundPlayback_Once(const void* inputBuffer, void* outputBuffer, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData);
	};
}