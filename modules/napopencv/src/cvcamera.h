#pragma once

// Local Includes
#include "cvadapter.h"

// External Includes
#include <glm/glm.hpp>
#include <atomic>
#include <nap/numeric.h>

namespace nap
{
	/**
	 * Configurable OpenCV camera settings.
	 */
	struct NAPAPI CVCameraSettings
	{
		bool	mAutoExposure	= true;		///< Property: 'AutoExposure' if auto exposure is turned on or off
		float	mBrightness		= 1.0f;		///< Property: 'Brightness' camera brightness
		float	mContrast		= 1.0f;		///< Property: 'Contrast' camera contrast
		float	mSaturation		= 1.0f;		///< Property: 'Saturation' camera saturation
		float	mGain			= 1.0f;		///< Property: 'Gain' camera gain
		float	mExposure		= 1.0f;		///< Property: 'Exposure' camera exposure

		/**
		 * @return the settings as a human readable string.
		 */
		std::string toString() const;
	};


	/**
	 * Captures a video stream from a web cam or other peripheral video capture device.
	 * The captured video frame is stored on the GPU when hardware acceleration is available (OpenCL).
	 * Otherwise the captured video frame is stored on the CPU.
	 * Camera settings can be provided on startup by enabling 'ApplySettings'.
	 * After startup the camera settings reflect the current state of the hardware.
	 */
	class NAPAPI CVCamera : public CVAdapter
	{
		RTTI_ENABLE(CVAdapter)
	public:

		/**
		 * Initialize this object after de-serialization
		 * @param errorState contains the error message when initialization fails
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Returns frame width in pixels.
		 * @return frame width in pixels.
		 */
		int getWidth() const;

		/**
		 * Returns frame height in pixels.
		 * @return frame height in pixels.
		 */
		int getHeight() const;

		/**
		 * Sets and applies new camera settings the next time a frame is captured.
		 * @param settings the camera settings to set and apply
		 * @param error contains the error message if the operation fails
		 * @return if the operation succeeded
		 */
		void setSettings(const nap::CVCameraSettings& settings);

		/**
		 * Returns the active, currently in use camera settings.
		 * Note that depending on your hardware this call can be slow.
		 * @return the current camera settings.
		 */
		void getSettings(nap::CVCameraSettings& settings);

		std::string			mCodec = "";					///< Property: 'Codec' optional video capture codec, for example: 'MJPG' or 'H264'. Leaving this empty defaults to regular codec. 
		bool				mConvertRGB = true;				///< Property: 'ConvertRGB' if the frame is converted into RGB
		bool				mFlipHorizontal = false;		///< Property: 'FlipHorizontal' flips the frame on the x-axis
		bool				mFlipVertical = false;			///< Property: 'FlipVertical' flips the frame on the y-axis
		bool				mResize = false;				///< Property: 'Resize' if the frame is resized to the specified 'Size' after capture
		glm::ivec2			mSize = { 1280, 720 };			///< Property: 'Size' frame size, only used when 'Resize' is turned on.
		bool				mApplySettings = false;			///< Property: 'ApplySettings' if the camera settings are applied on startup
		nap::uint			mDeviceIndex = 0;				///< Property: 'DeviceIndex' capture device index
		bool				mOverrideResolution = false;	///< Property: 'OverrideResolution' if the default camera resolution is used, when set to false the specified 'Resolution' is enforced.
		glm::ivec2			mResolution = { 640, 480 };		///< Property: 'Resolution' camera record resolution, only used when 'DefaultResolution' is turned off
		CVCameraSettings	mCameraSettings;				///< Property: 'Settings' all configurable camera settings
		bool				mShowDialog = false;			///< Property: 'ShowDialog' if the external camera settings dialog is shown on startup

	protected:
		virtual int getMatrixCount() override				{ return 1; }

		/**
		 * Opens the connection to the camera.
		 * @param captureDevice device to open
		 * @param api api back-end to use
		 * @param error contains the error if the opening operation fails
		 */
		virtual bool onOpen(cv::VideoCapture& captureDevice, int api, nap::utility::ErrorState& error) override;

		/**
		 * This method decodes and returns the just grabbed frame.
		 * @param captureDevice the device to capture the frame from.
		 * @param outFrame contains the new decoded frame
		 * @return if decoding succeeded.
		 */
		virtual CVFrame onRetrieve(cv::VideoCapture& captureDevice, utility::ErrorState& error) override;

	private:
		bool					mLocalSettings;
		CVFrame					mCaptureFrame					{ 1 };
		CVFrame					mOutputFrame					{ 1 };
	};
}
