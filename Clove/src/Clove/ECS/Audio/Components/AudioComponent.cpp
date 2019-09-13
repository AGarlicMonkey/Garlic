#include "clvpch.hpp"
#include "AudioComponent.hpp"

#include <portaudio.h>

namespace clv::ecs::aud{
	void AudioComponent::setSound(const clv::aud::Sound& sound){
		this->sound = sound;
	}

	void AudioComponent::play(PlaybackMode playback){
		if(sound.isValid()){
			requestedPlayback = playback;
		} else{
			CLV_LOG_ERROR("{0} called without valid sound. Please call setSound with an initialised Sound class");
		}
	}

	void AudioComponent::pause(){
		if(sound.isValid()){
			requestedStopMode = StopMode::pause;
		} else{
			CLV_LOG_ERROR("{0} called without valid sound. Please call setSound with an initialised Sound class");
		}
	}

	void AudioComponent::resume(){
		if(sound.isValid()){
			if(currentPlayback && !isPlaying()){
				requestedPlayback = currentPlayback;
			}
		} else{
			CLV_LOG_ERROR("{0} called without valid sound. Please call setSound with an initialised Sound class");
		}
	}

	void AudioComponent::stop(){
		if(sound.isValid()){
			requestedStopMode = StopMode::stop;
		} else{
			CLV_LOG_ERROR("{0} called without valid sound. Please call setSound with an initialised Sound class");
		}
	}

	bool AudioComponent::isPlaying(){
		return playing;
	}
}