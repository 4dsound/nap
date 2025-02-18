#include "napstatic.h"

#include <utility/module.h>
#include <audio/service/audioservice.h>
#include <audio/utility/audiotypes.h>
#include <nap/core.h>

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
		return false;
	audioService = core->getService<nap::audio::AudioService>();
	if (audioService == nullptr)
	{
		errorState.fail("No AudioService found");
		return false;
	}
	services = core->initializeServices(errorState);
	if (!services->initialized())
		return false;

	audioService->getNodeManager().setInputChannelCount(1);
	audioService->getNodeManager().setOutputChannelCount(2);

	std::vector<nap::rtti::FileLink> fileLinks;
	if (!core->getResourceManager()->loadJSON(app_structure, std::string(), fileLinks, errorState))
		return false;

	core->start();
	return true;
}


bool getError(char* buffer, int size)
{
	if (errorState.hasErrors())
	{
		auto str = errorState.toString();
		for (auto i = 0; i < std::min<int>(size, str.size()); ++i)
			buffer[i] = str[i];
		return true;
	}
	else return false;
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


void process(float* buffer, int length, int numChannels)
{
	int inputChannelCount = std::min<int>(audioService->getNodeManager().getInputChannelCount(), numChannels);
	int outputChannelCount = std::min<int>(audioService->getNodeManager().getOutputChannelCount(), numChannels);

	// Deinterleave
	float* inWritePtr[inputChannelCount];
	for (int channel = 0; channel < inputChannelCount; ++channel)
	{
		inWritePtr[channel] = inputBuffer[channel].data();
		int channelOffset = channel * length;
		for (int i = 0; i < length; ++i)
			inputBuffer[channel][i] = buffer[channelOffset + i];
	}

	float* outWritePtr[outputChannelCount];
	for (int channel = 0; channel < outputChannelCount; ++channel)
		outWritePtr[channel] = outputBuffer[channel].data();

	audioService->onAudioCallback(inWritePtr, outWritePtr, length);

	// Interleave
	for (int channel = 0; channel < outputChannelCount; ++channel)
	{
		int channelOffset = channel * length;
		for (int i = 0; i < length; ++i)
			buffer[channelOffset + i] = outWritePtr[channel][i];
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
