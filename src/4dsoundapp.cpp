#include "4dsoundapp.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/logger.h>
#include <inputrouter.h>
#include <perspcameracomponent.h>
#include <renderable2dtextcomponent.h>
#include <rendercomponent.h>

// Spatial includes.
#include <Spatial/MultiSpeaker/MultiSpeakerSetup.h>
#include <Spatial/Core/SpatialTypes.h>
#include <Spatial/Core/EnvironmentComponent.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SpatialSoundApp)
RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
    /**
     * Initialize all the resources and instances used for drawing
     * slowly migrating all functionality to NAP
     */
    bool SpatialSoundApp::init(utility::ErrorState& error)
    {
        // Retrieve services
        mRenderService	= getCore().getService<nap::RenderService>();
        mSceneService	= getCore().getService<nap::SceneService>();
        mInputService	= getCore().getService<nap::InputService>();
        mGuiService		= getCore().getService<nap::IMGuiService>();
        mSpatialService = getCore().getService<spatial::SpatialService>();
        mParameterService = getCore().getService<nap::ParameterService>();
        
        // Fetch the resource manager
        mResourceManager = getCore().getResourceManager();
        
        // Fetch the scene
        mScene = mResourceManager->findObject<Scene>("Scene");
        
        // Convert our path and load resources from file
        auto abspath = utility::getAbsolutePath("app_structure.json");
        if (!mResourceManager->loadFile(abspath, error))
            return false;
		
		
		// Get the gui window and make sure it's visible by moving it to the left of the screen
		mGuiWindow = mResourceManager->findObject<nap::RenderWindow>("GuiWindow");
		if (!error.check(mGuiWindow != nullptr, "unable to find gui window with name: %s", "GuiWindow"))
            return false;
		
		// Get the render window
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window");
        if (!error.check(mRenderWindow != nullptr, "unable to find render window with name: %s", "Window"))
            return false;
		
        mGuiWindow->setPosition(glm::vec2(10, mGuiWindow->getPosition()[1]));

        // Get the scene that contains our entities and components
        mScene = mResourceManager->findObject<Scene>("Scene");
        if (!error.check(mScene != nullptr, "unable to find scene with name: %s", "Scene"))
            return false;

		// Find the camera
		auto cameraEntity = mScene->findEntity("camera");
		mCamera = cameraEntity->findComponent<nap::PerspCameraComponentInstance>();

		// Get the default input router
        mDefaultInputRouter = mScene->findEntity("DefaultInputRouterEntity");

        // Set the environment script to the command line argument (if any)
        auto globalEntity = mScene->findEntity("global");
        auto environmentComponent = globalEntity->findComponent<spatial::EnvironmentComponentInstance>();
        if (!mCommandLineArgs.empty())
            environmentComponent->setScriptPath(mCommandLineArgs[0]);
        
        // All done!
        return true;
    }
    
    
    // Called when the window is updating
    void SpatialSoundApp::update(double deltaTime)
    {
        // Use a default input router to forward input events (recursively) to all input components in the default scene
        nap::DefaultInputRouter input_router(true);
        mInputService->processWindowEvents(*mRenderWindow, input_router, { &mScene->getRootEntity() });

		mGuiService->selectWindow(mRenderWindow);

		mGuiService->selectWindow(mGuiWindow);
    }


	void SpatialSoundApp::render()
	{
		mRenderService->beginFrame();

		if (mRenderService->beginRecording(*mGuiWindow))
		{
			mGuiWindow->beginRendering();
			mGuiService->draw();
			mGuiWindow->endRendering();
			mRenderService->endRecording();
		}

		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Begin render pass
			mRenderWindow->beginRendering();

			// Render GUI elements
			mGuiService->draw();

			// Stop render pass
			mRenderWindow->endRendering();

			// End recording
			mRenderService->endRecording();
		}

		// Proceed to next frame
		mRenderService->endFrame();
	}
    

    void SpatialSoundApp::windowMessageReceived(WindowEventPtr windowEvent)
    {
        mRenderService->addEvent(std::move(windowEvent));
    }
    
    
    void SpatialSoundApp::inputMessageReceived(InputEventPtr inputEvent)
    {
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());

			if (press_event->mKey == nap::EKeyCode::KEY_TAB)
			{
				mGuiWindowIsVisible = !mGuiWindowIsVisible;
				if (mGuiWindowIsVisible)
				{
					mGuiWindow->show();
				}
				else
				{
					mGuiWindow->hide();
					mRenderWindow->show();
				}
			}

			if (press_event->mKey == nap::EKeyCode::KEY_LCTRL || press_event->mKey == nap::EKeyCode::KEY_RCTRL)
				mCtrlKeyPressed = true;

			if (press_event->mKey == nap::EKeyCode::KEY_q)
				if (mCtrlKeyPressed)
					quit();
		}

		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyReleaseEvent)))
		{
			nap::KeyReleaseEvent* releaseEvent = static_cast<nap::KeyReleaseEvent*>(inputEvent.get());

			if (releaseEvent->mKey == nap::EKeyCode::KEY_LCTRL || releaseEvent->mKey == nap::EKeyCode::KEY_RCTRL)
				mCtrlKeyPressed = false;

		}

        mInputService->addEvent(std::move(inputEvent));
    }
    
    
    int SpatialSoundApp::shutdown()
    {
        utility::ErrorState errorState;
        if (!getCore().writeConfigFile(errorState))
            Logger::warn("Failed to write configuration file: %s", errorState.toString().c_str());
        return 0;
    }
    
}
