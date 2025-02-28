#include "napstatic.h"

#include <utility/module.h>
#include <audio/service/audioservice.h>
#include <audio/utility/audiotypes.h>
#include <nap/core.h>

#include <entity.h>

NAP_MODULE("napstatic", "0.4.0")

static std::unique_ptr<nap::Core> core;
static nap::utility::ErrorState errorState;
static nap::audio::AudioService* audioService = nullptr;
static nap::Core::ServicesHandle services;
static nap::audio::MultiSampleBuffer inputBuffer;
static nap::audio::MultiSampleBuffer outputBuffer;
static std::function<void(double)> updateFunction = [](double){};

bool initialize(const char* app_structure_c_str)
{
	std::string app_structure(app_structure_c_str);
	core = std::make_unique<nap::Core>();
	if (!core->initializeEngineWithoutProjectInfo(errorState))
	{
		nap::Logger::error(errorState.toString());
		return false;
	}
	audioService = core->getService<nap::audio::AudioService>();
	if (audioService == nullptr)
	{
		errorState.fail("No AudioService found");
		nap::Logger::error(errorState.toString());
		return false;
	}
	services = core->initializeServices(errorState);
	if (!services->initialized())
	{
		nap::Logger::error(errorState.toString());
		return false;
	}

	audioService->getNodeManager().setInputChannelCount(1);
	audioService->getNodeManager().setOutputChannelCount(2);

	std::vector<nap::rtti::FileLink> fileLinks;
	if (!core->getResourceManager()->loadJSON(app_structure, std::string(), fileLinks, errorState))
	{
		nap::Logger::error(errorState.toString());
		return false;
	}

	core->start();
	return true;
}



void setAudioSettings(int inputChannelCount, int outputChannelCount, int sampleRate, int bufferSize)
{
	audioService->getNodeManager().setInputChannelCount(inputChannelCount);
	audioService->getNodeManager().setOutputChannelCount(outputChannelCount);
	audioService->getNodeManager().setSampleRate(sampleRate);
	audioService->getNodeManager().setInternalBufferSize(bufferSize);
	inputBuffer.resize(audioService->getNodeManager().getInputChannelCount(), bufferSize);
	outputBuffer.resize(audioService->getNodeManager().getOutputChannelCount(), bufferSize);
}


// Note: the buffer is interleaved and 'length' is the length of the entire buffer.
void process(float* buffer, int length, int numChannels)
{
	// Assert that the length is as expected.
	assert(length == outputBuffer[0].size() * numChannels);
	
	int channelLength = length / numChannels;
	
	// Get write pointers.
	float* inWritePtr[numChannels];
	float* outWritePtr[numChannels];
	for (int channel = 0; channel < numChannels; ++channel)
	{
		inWritePtr[channel] = inputBuffer[channel].data();
		outWritePtr[channel] = outputBuffer[channel].data();
	}
	
	audioService->onAudioCallback(inWritePtr, outWritePtr, channelLength);
		
	// Deinterleave and set buffer
	int i = 0;
	for (int sample = 0; sample < channelLength; ++sample)
	{
		for(int channel = 0; channel < numChannels; ++channel)
		{
			buffer[i] = outputBuffer[channel][sample];
			i++;
		}
	}
	
}


void update()
{
	core->update(updateFunction);
}


void uninitialize()
{
	services.reset(nullptr);
}
