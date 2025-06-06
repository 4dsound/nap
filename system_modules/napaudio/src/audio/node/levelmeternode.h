/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Std includes
#include <atomic>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/utility/dirtyflag.h>
#include <audio/utility/safeptr.h>
#include <audio/core/process.h>

namespace nap
{
	namespace audio
	{

		// Forward declarations
		class AudioService;

		/**
		 * Node to measure the amplitude level of an audio signal.
		 * Can be used for VU meters or envelope followers for example.
		 * Can switch between measuring peaks of the signal or the root mean square.
		 */
		class NAPAPI LevelMeterNode : public Node
		{
		public:
			enum class EType
			{
				PEAK, RMS
			};

			/**
			 * @param nodeManager: the node manager
			 * @param analysisWindowSize: the time window in milliseconds that will be used to generate one single output value. Also the period that corresponds to the analysis frequency.
			 */
			LevelMeterNode(NodeManager& nodeManager, TimeValue analysisWindowSize = 10);

			virtual ~LevelMeterNode();

			InputPin input = {this}; /**< The input for the audio signal that will be analyzed. */

			OutputPin output = { this }; /**< If this output is connected the LevelMeterNode will output its input. */

			/**
			 * @return: The current level of the analyzed signal.
			 */
			float getLevel();

			/**
			 * Set the Type of the analysis. PEAK means the highest absolute amplitude within the analyzed window will be output. RMS means the root mean square of all values within the analyzed window will be output.
			 */
			void setType(EType type) { mType = type; }

			// Inherited from Node
			void process() override;
      		void sampleRateChanged(float sampleRate) override;

		private:
			int mIndex = 0; // Current write index of the buffer being analyzed.
			EType mType = EType::RMS; // Algorithm currently being used to calculate the output level value from one buffer.
			TimeValue mAnalysisWindowSize = 0.f;
			int mWindowSizeInSamples = 0;
			std::atomic<float> mValue = 0.f; // Calculated output level value
			
			SampleBuffer mSquaredBuffer;
			float mSquaredSum = 0.f;
			
			float mPeak = 0.f;
			float mPeakTemp = 0.f;

			bool mRootProcess = false;
		};

	}
}
