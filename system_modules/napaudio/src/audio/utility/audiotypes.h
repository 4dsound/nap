/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// std library includes
#include <vector>

// nap includes
#include <utility/dllexport.h>
#include <nap/numeric.h>
#include <rtti/typeinfo.h>

namespace nap
{
	
	namespace audio
	{
		
		// Note: in this files typedefs are being used instead of a using statement
		// This is because the using statement cannot be used in combination with the NAPAPI macro
		
		
		/**
		 * Value of a single audio sample
		 * Change this to double to build with double precision sample calculation
		 */
		using SampleValue = float;
		
		
		/**
		 * A buffer of samples
		 */
		using SampleBuffer = std::vector<SampleValue>;
		
		
		/**
		 * A collection of sample buffers, one for each channel to represent multichannel audio.
		 */
		class NAPAPI MultiSampleBuffer
		{
			RTTI_ENABLE()
		public:
			MultiSampleBuffer() = default;
			
			/**
			 * @param channelCount: number of channels in this buffer
			 * @param size: size of the buffer in samples
			 */
			MultiSampleBuffer(std::size_t channelCount, std::size_t size)
			{
				resize(channelCount, size);
			}
			
			/**
			 * Used to access the samples in the buffer
			 * example: myBuffer[channelNumber][sampleIndex]
			 */
			SampleBuffer& operator[](std::size_t index)
			{
				return channels[index];
			}
			
			/**
			 * Used to access the samples in the buffer
			 * example: myBuffer[channelNumber][sampleIndex]
			 */
			const SampleBuffer& operator[](std::size_t index) const
			{
				return channels[index];
			}

			/**
			 * @return: number of channels in the buffer
			 */
			std::size_t getChannelCount() const { return channels.size(); }
			
			/**
			 * @return: the size of the buffer in samples
			 */
			std::size_t getSize() const { return channels.empty() ? 0 : channels.front().size(); }
			
			/**
			 * Resize the buffer
			 * @param channelCount: new number of channels
			 * @param size: new size in samples
			 */
			void resize(std::size_t channelCount, std::size_t size, SampleValue value = 0.f)
			{
				channels.resize(channelCount);
				for (auto& channel : channels)
					channel.resize(size, value);
			}
			
			/**
			 * Reserve capacity of the buffer in memory to prevent repeated memory allocation
			 * @param channelCount: new number of channels capacity
			 * @param size: new size in samples capacity
			 */
			void reserve(std::size_t channelCount, std::size_t size)
			{
				channels.reserve(channelCount);
				for (auto& channel : channels)
					channel.reserve(size);
			}
			
			/**
			 * Clear the content of the buffer.
			 */
			void clear()
			{
				channels.clear();
			}

			/**
			 * Moves a channel of audio data into a new channel of the MultiSampleBuffer.
			 * Doesn't allocate or deallocate sample data.
			 * @param buffer The buffer to move, will be emptied after the call.
			 */
			void addChannel(SampleBuffer&& buffer)
			{
				channels.emplace_back(std::move(buffer));
			}

			/**
			 * Moves the last channel out of the MultiSampleBuffer and returns it.
			 * Doesn't allocate or deallocate sample data.
			 * @return The moved last channel.
			 */
			SampleBuffer popChannel()
			{
				SampleBuffer result = std::move(channels.back());
				channels.pop_back();
				return std::move(result);
			}

		private:
			std::vector<SampleBuffer> channels;
		};


		/**
		 * Convenience type that behaves like a SampleBuffer but can also be a nullptr behind the scenes.
		 */
		struct OptionalSampleBuffer
		{
			/**
			 * Constructor
			 * @param bufferPtr The buffer if one is available, nullptr if not.
			 */
			OptionalSampleBuffer(SampleBuffer* bufferPtr) { mPtr = bufferPtr; }

			/**
			 * Set to a buffer or to nullptr.
			 * @param bufferPtr The buffer if one is available, nullptr if not.
			 */
			void set(SampleBuffer* bufferPtr) { mPtr = bufferPtr; }

			/**
			 * Returns contents of the buffer if one is present, zeroes if not.
			 * @param index Index in the buffer.
			 * @return Sample at index, zero if no buffer is available.
			 */
			SampleValue operator[](int index)
			{
				if (mPtr == nullptr)
					return 0.f;
				else
					return (*mPtr)[index];
			}

			/**
			 * @return Size of the buffer if one is present, zero if not.
			 */
			int size() const { return mPtr != nullptr ? mPtr->size() : 0; }

			bool operator==(const std::nullptr_t) const { return mPtr == nullptr; }
			bool operator!=(const std::nullptr_t) const { return mPtr != nullptr; }

			SampleBuffer* get() { return mPtr; }
			const SampleBuffer* get() const { return mPtr; }

		private:
			SampleBuffer* mPtr = nullptr;
		};
		
		
		/**
		 * Value of control parameter
		 * Change this to double to build with double precision sample calculation
		 */
		using ControllerValue = float;
		
		
		/**
		 * Time value in milliseconds
		 */
		using TimeValue = float;
		
		
		/**
		 * Time value in samples
		 */
		using DiscreteTimeValue = nap::uint64;
		
		
		/**
		 * Mode to calculate a ramp from one value to another in a certain amount of steps
		 */
		enum RampMode
		{
			Linear, Exponential
		};
		
		
	}
	
}
