/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "parameterrecordingapp.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <renderablemeshcomponent.h>
#include <renderable2dtextcomponent.h>
#include <orthocameracomponent.h>
#include <mathutils.h>
#include <scene.h>
#include <perspcameracomponent.h>
#include <inputrouter.h>
#include <uniforminstance.h>
#include <imguiutils.h>

#include <SimpleJSONExporting.h>
#include <SequenceExporting.h>

// Register this application with RTTI, this is required by the AppRunner to 
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterRecordingApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool ParameterRecordingApp::init(utility::ErrorState& error)
	{		
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();

		// Get resource manager
		mResourceManager = getCore().getResourceManager();

		// Extract loaded resources
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window0");

		// Sample default color values from loaded color palette
		mColorTwo = mGuiService->getPalette().mHighlightColor1.convert<RGBColorFloat>();
		mColorOne = { mColorTwo[0] * 0.9f, mColorTwo[1] * 0.9f, mColorTwo[2] };
		mHaloColor = mGuiService->getPalette().mFront4Color.convert<RGBColorFloat>();
		mTextColor = mGuiService->getPalette().mFront4Color.convert<RGBColorFloat>();
				
		mParameterGUI = mResourceManager->findObject<ParameterGUI>("ParameterGUI");
		if (!error.check(mParameterGUI != nullptr, "unable to find parameter GUI"))
			return false;
		
		mParameterGroupRecorder = mResourceManager->findObject<ParameterGroupRecorder>("ParametersRecorder");
		if (!error.check(mParameterGroupRecorder != nullptr, "unable to find ParametersRecorder"))
			return false;
		
		capFramerate(true);		

		return true;
	}
	
	
	/**
	 * Forward all the received input messages to the camera input components.
	 * The camera has two input components: KeyInputComponent and PointerInputComponent.
	 * The key input component receives key events, the pointer input component receives pointer events.
	 */
	void ParameterRecordingApp::update(double deltaTime)
	{
		// Create an input router, the default one forwards messages to mouse and keyboard input components
		nap::DefaultInputRouter input_router;
		
		// Setup GUI
		ImGui::Begin("Controls");
		
		// Show all parameters
		mParameterGUI->show(false);
		
		// Start / stop recording button
		
		if(mParameterGroupRecorder->isRecording())
		{
			if(ImGui::Button("Stop recording"))
				mParameterGroupRecorder->stopRecording();
		}
		else
		{
			if(ImGui::Button("Start recording"))
				mParameterGroupRecorder->startRecording();
			
			if(ImGui::Button("Save sequence"))
			{
				utility::ErrorState e;
				exportParameterGroupRecordingsAsSequence(*mParameterGroupRecorder, "sequenceExport.json", e);
				exportParameterGroupRecordingsAsSimpleJSON(*mParameterGroupRecorder, "simpleJSONExport.json", e);
			}
		}
		
		// Show framerate
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		
		ImGui::End();

	}

	
	/**
	 * Renders the world and text.
	 */
	void ParameterRecordingApp::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Begin recording the render commands for the main render window
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Begin the render pass
			mRenderWindow->beginRendering();

			// Draw our GUI
			mGuiService->draw();
			
			// End the render pass
			mRenderWindow->endRendering();

			// End recording
			mRenderService->endRecording();
		}

		// Signal the ending of the frame
		mRenderService->endFrame();
	}
	
	
	/**
	 * Occurs when the event handler receives a window message.
	 * You generally give it to the render service which in turn forwards it to the right internal window. 
	 * On the next update the render service automatically processes all window events. 
	 * If you want to listen to specific events associated with a window it's best to listen to a window's mWindowEvent signal
	 */
	void ParameterRecordingApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	 * Called by the app loop. It's best to forward messages to the input service for further processing later on
	 * In this case we also check if we need to toggle full-screen or exit the running app
	 */
	void ParameterRecordingApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		// Escape the loop when esc is pressed
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();

			// If 'f' is pressed toggle fullscreen
			if (press_event->mKey == nap::EKeyCode::KEY_f)
			{
				mRenderWindow->toggleFullscreen();
			}
		}

		mInputService->addEvent(std::move(inputEvent));
	}


	int ParameterRecordingApp::shutdown()
	{
		return 0;
	}
}
