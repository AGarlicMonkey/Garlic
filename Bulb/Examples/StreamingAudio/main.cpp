#include <Bulb/Bulb.hpp>

static clv::AudioFormat getBufferFormat(const blb::aud::SoundFile& file) {
    const bool is16 = file.getFormat() == blb::aud::SoundFile::Format::s16;

    if(file.getChannels() == 1) {
        return is16 ? clv::AudioFormat::Mono16 : clv::AudioFormat::Mono8;
    } else {
        return is16 ? clv::AudioFormat::Stereo16 : clv::AudioFormat::Stereo8;
    }
}

int main() {
    blb::aud::SoundFile file{ SOURCE_DIR "/countdown.wav" };

    auto audioFactory = clv::createAudioFactory(clv::AudioAPI::OpenAl);

    std::shared_ptr<clv::AudioBuffer> bufferA = audioFactory->createAudioBuffer();
    std::shared_ptr<clv::AudioBuffer> bufferB = audioFactory->createAudioBuffer();

    auto audioSource = audioFactory->createAudioSource();
    
    //Stream with 2 buffers each having a quater of the file
    const uint32_t framesPerBuffer = file.getFrames() / 4;

    //Get some data set up initially
    {
        auto [bufferAData, bufferASize] = file.read(framesPerBuffer);
        file.seek(blb::aud::SoundFile::SeekPosition::Beginning, framesPerBuffer);
        auto [bufferBData, bufferBSize] = file.read(framesPerBuffer);

        //Map buffers
        {
            clv::AudioDataDescriptor bufferData{};
            bufferData.format     = getBufferFormat(file);
            bufferData.sampleRate = file.getSampleRate();
            bufferData.data       = bufferAData;
            bufferData.dataSize   = bufferASize;
            bufferA->map(std::move(bufferData));
        }
        {
            clv::AudioDataDescriptor bufferData{};
            bufferData.format     = getBufferFormat(file);
            bufferData.sampleRate = file.getSampleRate();
            bufferData.data       = bufferBData;
            bufferData.dataSize   = bufferBSize;
            bufferB->map(std::move(bufferData));
        }

        delete bufferAData;
        delete bufferBData;
    }

    audioSource->queueBuffers({ bufferA, bufferB });

    audioSource->play();

    auto prevFrameTime = std::chrono::system_clock::now();
    while(true) {
        auto currFrameTime                        = std::chrono::system_clock::now();
        std::chrono::duration<float> deltaSeconds = currFrameTime - prevFrameTime;
        prevFrameTime                             = currFrameTime;

        //Update buffers
        uint32_t numProcessed = audioSource->getNumBuffersProcessed();
        if(numProcessed > 0) {
            while(numProcessed-- > 0) {
                auto buffer = audioSource->unQueueBuffers(1)[0];

                file.seek(blb::aud::SoundFile::SeekPosition::Current, framesPerBuffer);
                auto [data, size] = file.read(framesPerBuffer);

                clv::AudioDataDescriptor bufferData{};
                bufferData.format     = getBufferFormat(file);
                bufferData.sampleRate = file.getSampleRate();
                bufferData.data       = data;
                bufferData.dataSize   = size;
                buffer->map(std::move(bufferData));

                audioSource->queueBuffers({ buffer });
            }
        }

        static float total = 0.0f;
        total += deltaSeconds.count();

        //Stop after 15 seconds
        if(total >= 30.0f) {
            break;
        }
    }

    audioSource->unQueueBuffers(2);

    return 0;
}