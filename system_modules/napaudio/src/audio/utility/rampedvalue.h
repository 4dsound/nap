/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Std includes
#include <atomic>
#include <iostream>
#include <assert.h>

// Nap includes
#include <nap/signalslot.h>
#include <nap/logger.h>

// Audo includes
#include <audio/utility/audiotypes.h>
#include <audio/utility/dirtyflag.h>
#include <cmath>

namespace nap
{
	namespace audio
	{

		/**
		 * Used to make linear or exponential ramps up and down of a value in steps.
		 * The length of the ramp and the kind of ramp can be specified for each ramp.
		 */
		template<typename T>
		class RampedValue
		{
		public:
			inline static constexpr T smallestFactor = 0.0001f;

		public:
			RampedValue(const T& initValue) : mValue(initValue) 	{ }


			/**
			 * Stop the current ramp and set a value directly
			 */
			void setValue(const T& value)
			{
				stop();
				mValue = value;
			}

			/**
			 * Start a ramp
			 * @param destination the finishing value
			 * @param stepCount the number of steps
			 * @param mode the ramp interpolation mode
			 */
			void ramp(const T& destination, int stepCount, RampMode mode = RampMode::Linear)
			{
				assert(stepCount >= 0);

				mDestination.store(destination);
				mStepCount.store(stepCount);
				mRampMode.store(mode);
				
				updateRamp();
			}

			/**
			 * Stop the current ramp.
			 */
			void stop() { mStepCounter = 0; }

			/**
			 * Take the next step in the current ramp.
			 * Should only be called from the audio thread.
			 */
			T getNextValue()
			{
				RampMode rampMode = mRampMode.load();
				
				if (mStepCounter > 0) {
					switch (rampMode) {
						case RampMode::Linear:
							mValue = mValue + mIncrement;
							break;
						case RampMode::Exponential:
							mValue = mValue * mFactor;
							break;
					}
					mStepCounter--;
					if (mStepCounter == 0) {
						if (rampMode == RampMode::Exponential && mDestinationZero)
							mValue = 0;
						else
							mValue = mDestination.load();
						destinationReachedSignal(mValue);
					}
				}

				return mValue;
			}

			/**
			 * @return the current value.
			 * Should only be called from the audio thread
			 */
			T getValue() const { return mValue; }

			/**
			 * @return true when currently playing a ramp.
			 * Should only be called from the audio thread.
			 */
			bool isRamping() const { return mStepCounter > 0; }

			/**
			 * Signal emitted when the destination of a ramp has been reached.
			 */
			nap::Signal<T> destinationReachedSignal;

		private:
			void updateRamp()
			{
				T destination = mDestination.load();
				int stepCount = mStepCount.load();
				
				// if there are zero steps we reach the destination of the ramp immediately
				if (stepCount <= 0)
				{
					mStepCounter = 0;
					mValue = destination;
					destinationReachedSignal(mValue);
					return;
				}

				mStepCounter = mStepCount;
				
				switch (mRampMode.load())
				{
					case RampMode::Linear:
						mIncrement = (destination - mValue) / T(stepCount);
						break;

					case RampMode::Exponential:
						// avoid divisions by zero by avoiding mValue = 0
						if (mValue == 0)
							mValue = destination * smallestFactor; // this is a 140dB ramp up from mValue to mDestination
						
						// avoid divisions by zero by avoiding mDestination = 0
						if (destination == 0)
						{
							destination = mValue * smallestFactor; // this is a 140 dB ramp down from mValue to mDestination
							mDestinationZero = true;
						}
						else
							mDestinationZero = false;

						// calculate the increment factor
						mFactor = pow(double(destination / mValue), double(1.0 / stepCount));
						break;
				}
			}

		private:
			T mValue; // Value that is being controlled by this object.

			union
			{
				T mIncrement; // Increment value per step of the current ramp when mode is linear.
				T mFactor; // Factor value per step of the current ramp when mode is exponential.
			};
			std::atomic<T> mDestination = { 0 }; // Destination value of the current ramp.
			std::atomic<int> mStepCount = { 0 }; // Number of steps in the ramp.
			int mStepCounter = 0; // Current step index, 0 means at destination
			std::atomic<RampMode> mRampMode = {RampMode::Linear}; // The mode of the current ramp
			bool mDestinationZero = false; // In case of a linear ramp this indicates wether the destination value needs to be rounded to zero.
		};


	}
}
