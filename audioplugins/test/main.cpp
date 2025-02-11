//
// Created by Stijn on 06/02/2025.
//

#include <napstatic.h>
#include <utility/fileutils.h>
#include <nap/logger.h>

#define str(s) #s
#define xstr(s) str(s)

int main(int argc, char *argv[])
{
	std::string app_structure_path = xstr(APP_STRUCTURE_PATH);
	std::string data_dir = xstr(DATA_DIR);
	std::string app_structure;
	nap::utility::ErrorState errorState;
	if (!nap::utility::readFileToString(app_structure_path, app_structure, errorState))
	{
		errorState.fail("Failed to load file: %s", app_structure_path.c_str());
		return false;
	}

	if (!initialize(app_structure.c_str()))
	{
		char text[2048];
		getError(text, 2048);
		nap::Logger::error(text);
		return false;
	}

	int sampleRate = 44100;
	int bufferSize = 256;
	int channelCount = 2;
	setAudioSettings(0, channelCount, sampleRate, bufferSize);

	std::vector<float> buffer;
	buffer.resize(channelCount * bufferSize);
	int i = 0;
	for (i = 0; i < 256; ++i)
	{
		process(buffer.data(), bufferSize, channelCount);
		update();
	}
	nap::Logger::info("Successfully processed %i buffers of data", i);
	uninitialize();
}