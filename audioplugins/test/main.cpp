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
	if (argc > 1)
		app_structure_path = argv[1];
	nap::utility::ErrorState errorState;
	if (!nap::utility::readFileToString(app_structure_path, app_structure, errorState))
	{
		errorState.fail("Failed to load file: %s", app_structure_path.c_str());
		nap::Logger::error(errorState.toString());
		return -1;
	}

	if (!initialize(app_structure.c_str()))
	{
		return -1;
	}

	int sampleRate = 44100;
	int bufferSize = 1024;
	int channelCount = 2;
	setAudioSettings(channelCount, channelCount, sampleRate, bufferSize);
	
	std::vector<float> buffer;
	buffer.resize(channelCount * bufferSize);
	int i = 0;
	for (i = 0; i < 256; ++i)
	{
		process(buffer.data(), channelCount * bufferSize, channelCount);
		update();
	}
	nap::Logger::info("Successfully processed %i buffers of data", i);
	uninitialize();
}
