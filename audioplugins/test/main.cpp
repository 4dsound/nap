//
// Created by Stijn on 06/02/2025.
//

#include <nap/core.h>
#include <nap/logger.h>

int main(int argc, char *argv[])
{
	// Create core
	nap::Core core;
	nap::utility::ErrorState errorState;
	core.initializeEngineWithoutProjectInfo(errorState);
	nap::Logger::info("Success");
}