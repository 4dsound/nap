#pragma once

bool initialize(const char* app_structure_c_str);
bool getError(char* buffer, int size);
void setAudioSettings(int inputChannelCount, int outputChannelCount, int sampleRate, int bufferSize);
void process(float* buffer, int length, int numChannels);
void update();
void uninitialize();