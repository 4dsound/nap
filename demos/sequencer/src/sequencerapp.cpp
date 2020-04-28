// Local Includes
#include "sequencerapp.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/logger.h>
#include <inputrouter.h>
#include <napcolors.h>
#include <sequenceeventreceiver.h>
#include <sequenceevent.h>

namespace nap 
{    
    bool SequencerApp::init(utility::ErrorState& error)
    {
		// Retrieve services
		mRenderService		= getCore().getService<nap::RenderService>();
		mSceneService		= getCore().getService<nap::SceneService>();
		mInputService		= getCore().getService<nap::InputService>();
		mGuiService			= getCore().getService<nap::IMGuiService>();
		mParameterService	= getCore().getService<nap::ParameterService>();

		// Fetch the resource manager
        mResourceManager = getCore().getResourceManager();
		
		// Fetch the scene
		mScene = mResourceManager->findObject<Scene>("Scene");

		// Convert our path and load resources from file
        auto abspath = utility::getAbsolutePath(mFilename);
        nap::Logger::info("Loading: %s", abspath.c_str());
        if (!mResourceManager->loadFile(mFilename, error))
            return false;

		// Get the render window
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window");
		if (!error.check(mRenderWindow != nullptr, "unable to find render window with name: %s", "Window"))
			return false;

		// Get the scene that contains our entities and components
		mScene = mResourceManager->findObject<Scene>("Scene");
		if (!error.check(mScene != nullptr, "unable to find scene with name: %s", "Scene"))
			return false;

		mSequenceEditorGUI = mResourceManager->findObject<SequenceEditorGUI>("SequenceEditorGUI");
		if (!error.check(mSequenceEditorGUI != nullptr, "unable to find SequenceEditorGUI with name: %s", "SequenceEditorGUI"))
			return false;

		mSequencePlayerGUI = mResourceManager->findObject<SequencePlayerGUI>("SequencePlayerGUI");
		if (!error.check(mSequencePlayerGUI != nullptr, "unable to find SequencePlayerGUI with name: %s", "SequenceEditorGUI"))
			return false;

		mParameterGroup = mResourceManager->findObject<ParameterGroup>("ParameterGroup");
		if (!error.check(mParameterGroup != nullptr, "unable to find ParameterGroup with name: %s", "ParameterGroup"))
			return false;

		const ObjectPtr<SequenceEventReceiver> eventReceiver = mResourceManager->findObject<SequenceEventReceiver>("SequenceEventReceiver");
		if (!error.check(eventReceiver != nullptr, "unable to find SequenceEventReceiver with name: %s", "SequenceEventReceiver"))
			return false;

		eventReceiver->mSignal.connect([](const SequenceEvent &event)
		{
			if (event.get_type().is_derived_from(RTTI_OF(SequenceEventString)))
			{
				const SequenceEventString& eventString = static_cast<const SequenceEventString&>(event);
				nap::Logger::info("Event received with message : %s", eventString.getMessage().c_str());
			}
		});

		// Create the parameter GUI, automatically shows a group of parameters in a window
		mParameterGUI = std::make_unique<ParameterGUI>(*mParameterService);

		// All done!
        return true;
    }

	
	/**
	 * Forwards the received mouse and keyboard input events and shows the sequencer gui elements,
	 * together with some general application information and the parameters.
	 */
	void SequencerApp::update(double deltaTime)
	{
		// Use a default input router to forward input events (recursively) to all input components in the default scene
		nap::DefaultInputRouter input_router(true);
		mInputService->processWindowEvents(*mRenderWindow, input_router, { &mScene->getRootEntity() });

		// Show parameters
		mParameterGUI->show(mParameterGroup.get(), true);

		// Draw general application information
		ImGui::Begin("Information");
		ImGui::SameLine();
		getCore().getFramerate();
		ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(guicolors::red), "%.3f ms/frame (%.1f FPS)", 1000.0f / getCore().getFramerate(), getCore().getFramerate());
		ImGui::End();

		// Show sequence editor gui
		mSequenceEditorGUI->show();

		// Show sequence player gui
		mSequencePlayerGUI->show();
	}


    // Called when the window is going to render
	void SequencerApp::render()
	{
		// Activate current window for drawing
		mRenderWindow->makeActive();

		// Clear back-buffer
		mRenderService->clearRenderTarget(mRenderWindow->getBackbuffer());

		// Draw GUI to screen
		mGuiService->draw();

		// Swap screen buffers
		mRenderWindow->swap();
    }


    void SequencerApp::windowMessageReceived(WindowEventPtr windowEvent)
    {
		mRenderService->addEvent(std::move(windowEvent));
    }


    void SequencerApp::inputMessageReceived(InputEventPtr inputEvent)
    {
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			// Exit when esc is pressed
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();

			// If 'f' is pressed toggle fullscreen
			if (press_event->mKey == nap::EKeyCode::KEY_f)
				mRenderWindow->toggleFullscreen();
		}

		// Forward to input service
		mInputService->addEvent(std::move(inputEvent));
    }


    int SequencerApp::shutdown()
    {
		return 0;
    }
}