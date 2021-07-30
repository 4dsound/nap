#include "4dsoundapp.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/logger.h>
#include <inputrouter.h>
#include <perspcameracomponent.h>
#include <rendercomponent.h>
#include <audio/utility/audiofunctions.h>
#include <Gui/GuiFunctions.h>
#include <imgui/imgui_internal.h>
#include <imguiutils.h>
#include <videoplayer.h>

// Spatial includes.
#include <Spatial/Core/EnvironmentComponent.h>
#include <Spatial/Gui/GuiStyle.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SpatialSoundApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{

	static void getRenderableComponentsRecursive(EntityInstance& entity, std::vector<RenderableComponentInstance*>& components)
	{
		std::vector<RenderableComponentInstance*> locals;
		entity.getComponentsOfType<RenderableComponentInstance>(locals);
		for (auto& comp : locals)
			components.emplace_back(comp);
		for (auto& entity : entity.getChildren())
			getRenderableComponentsRecursive(*entity, components);
	}


	template <typename T>
	T* findComponentInScene(Scene& scene, const std::string& entityID, utility::ErrorState& errorState)
	{
		auto entity = scene.findEntity(entityID);
		if (!errorState.check(entity != nullptr, "Unable to find entity %s", entityID.c_str()))
			return nullptr;
		auto result = entity->template findComponent<T>();
		if (!errorState.check(result != nullptr, "Unable to find %s in %s", rtti::TypeInfo::get<T>().get_name().data(), entityID.c_str()))
			return nullptr;
		return result;
	}

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

        // Fetch the resource manager
        mResourceManager = getCore().getResourceManager();
        
        // Fetch the scene
        mScene = mResourceManager->findObject<Scene>("Scene");
        
        // load resources from file
        if (!mResourceManager->loadFile(mFileName, error))
            return false;
		
		// Get the render window
		mWindow = mResourceManager->findObject<nap::RenderWindow>("Window");
        if (!error.check(mWindow != nullptr, "unable to find render window with name: %s", "Window"))
            return false;
		mWindow->hide(); // Hide on startup

		// Get the detached gui window
		mSecondaryWindow = mResourceManager->findObject<nap::RenderWindow>("SecondaryWindow");
		if (!error.check(mSecondaryWindow != nullptr, "unable to find gui window with name: %s", "SecondaryWindow"))
			return false;

		// Get the startup window
		mStartupWindow = mResourceManager->findObject<nap::RenderWindow>("StartupWindow");
		if (!error.check(mStartupWindow != nullptr, "unable to find window with name: %s", "StartupWindow"))
			return false;
		mStartupWindow->show(); // Show on startup

		// Get the scene that contains our entities and components
        mScene = mResourceManager->findObject<Scene>("Scene");
        if (!error.check(mScene != nullptr, "unable to find scene with name: %s", "Scene"))
            return false;

		// Find the camera
		mCamera = findComponentInScene<PerspCameraComponentInstance>(*mScene, "camera", error);
		if (mCamera == nullptr)
			return false;

		// Find floor wireframe
		mFloorWireFrame = findComponentInScene<RenderableComponentInstance>(*mScene, "MonitorFloorWireFrame", error);
		if (mFloorWireFrame == nullptr)
			return false;

		// Find floor
		mFloor = findComponentInScene<RenderableComponentInstance>(*mScene, "MonitorFloor", error);
		if (mFloor == nullptr)
			return false;

		// Find axes helpers
		mAxesHelpers = findComponentInScene<RenderableComponentInstance>(*mScene, "AxesHelper", error);
		if (mAxesHelpers == nullptr)
			return false;

		// Find speaker visualizations
		mSatellites = findComponentInScene<RenderableComponentInstance>(*mScene, "SatellitesVisualization", error);
		if (mSatellites == nullptr)
			return false;
		mSubs = findComponentInScene<RenderableComponentInstance>(*mScene, "SubsVisualization", error);
		if (mSubs == nullptr)
			return false;
        mSideFills = findComponentInScene<RenderableComponentInstance>(*mScene, "SideFillsVisualization", error);
        if (mSideFills == nullptr)
            return false;

		// Find text overlay controller
		mTextOverlayController = findComponentInScene<nap::spatial::TextOverlayControllerInstance>(*mScene, "MonitorTextOverlay", error);
		if (mTextOverlayController == nullptr)
			return false;

		// Find the environment
		mEnvironment = findComponentInScene<spatial::EnvironmentComponentInstance>(*mScene, "Environment", error);
		if (mEnvironment == nullptr)
			return false;

		// Set the environment script to the command line argument (if any)
        if (!mCommandLineArgs.empty())
			mEnvironment->setScriptPath(mCommandLineArgs[0]);

		mGuiWindow = mResourceManager->findObject<gui::Gui>("GuiWindow");
		if (!error.check(mGuiWindow != nullptr, "GuiWindow not found"))
			return false;

		mDetachedGuiWindow = mResourceManager->findObject<gui::Gui>("DetachedGuiWindow");
		if (!error.check(mDetachedGuiWindow != nullptr, "DetachedGuiWindow not found"))
			return false;

		// Get the splash screen gui
		mSplashScreenGui = mResourceManager->findObject<gui::Gui>("SplashScreenGui");
//        mSplashScreenGui->show(); // show on startup
		if (!error.check(mSplashScreenGui != nullptr, "Splash Screen Gui not found"))
			return false;

		mMonitorGui = mResourceManager->findObject<gui::Gui>("MonitorGui");
		if (!error.check(mMonitorGui != nullptr, "Monitor Gui not found"))
			return false;

		mMonitorController = mResourceManager->findObject<spatial::MonitorController>("MonitorController");
		if (!error.check(mMonitorController != nullptr, "MonitorController not found"))
			return false;

		mEnvironmentStateMachine = mResourceManager->findObject<StateMachine>("EnvironmentState");
		if (!error.check(mEnvironmentStateMachine != nullptr, "EnvironmentState not found"))
			return false;

		mEnvironmentStartupState = mResourceManager->findObject<StateMachine::State>("startupState");
		if (!error.check(mEnvironmentStateMachine != nullptr, "EnvironmentStateMachine state startupState not found"))
			return false;

		// Apply hard-coded ImGui style to both windows
		spatial::GuiStyle guiStyle;
		guiStyle.apply(&mGuiService->getContext(mSecondaryWindow)->Style);
		guiStyle.apply(&mGuiService->getContext(mWindow)->Style);
        guiStyle.apply(&mGuiService->getContext(mStartupWindow)->Style);


		// Turn framerate capping on
		capFramerate(true);

		// All done!
        return true;
    }



    // Called when the window is updating
    void SpatialSoundApp::update(double deltaTime)
    {
		// Use a default input router to forward input events (recursively) to all input components in the default scene
		nap::DefaultInputRouter input_router(true);
		mInputService->processWindowEvents(*mWindow, input_router, { &mScene->getRootEntity() });

		// Don't show GUIs while in loading state. Doing so will lock the main thread as the control thread is busy
		if (mEnvironmentStateMachine->getCurrentState().get() == mEnvironmentStartupState.get())
		{
            mSplashScreenGui->show();
			return;
		}
        
        if (mStartupWindowVisible)
        {
            mStartupWindow->hide();
            mStartupWindowVisible = false;
        }
        if (!mPrimaryWindowVisible)
        {
            mWindow->show();
            mPrimaryWindowVisible = true;
        }

		// Show the Gui
		if (mGuiWindow->mOpen)
			mGuiWindow->show();

		if (mDetachedGuiWindow->mOpen)
		{
			if (!mSecondaryWindowVisible)
			{
				mSecondaryWindow->show();
				mSecondaryWindowVisible = true;
			}
			mDetachedGuiWindow->show();
		}
		else {
			if (mSecondaryWindowVisible)
			{
				mSecondaryWindow->hide();
				mSecondaryWindowVisible = false;
			}
		}
        
		// Show the Monitor Gui
		if (mMonitorController->isRenderingEnabled())
			mMonitorGui->show();

		// Control framerate
		if (mMonitorController->getFrameRate() != getRequestedFramerate())
			setFramerate(mMonitorController->getFrameRate());
    }


	void SpatialSoundApp::render()
	{
		mRenderService->beginFrame();

		if (mEnvironmentStateMachine->getCurrentState().get() == mEnvironmentStartupState.get())
		{
                        
            // Render the floor wireframe.
			if (mRenderService->beginRecording(*mStartupWindow))
			{
				mStartupWindow->beginRendering();
				mGuiService->draw();
				mStartupWindow->endRendering();
				mRenderService->endRecording();
			}
		}
		else {
            
            // Render the secondary window gui.
			if (mRenderService->beginRecording(*mSecondaryWindow))
			{
				mSecondaryWindow->beginRendering();
				mGuiService->draw();
				mSecondaryWindow->endRendering();
				mRenderService->endRecording();
			}

            // Render the primary window monitor and gui.
			if (mRenderService->beginRecording(*mWindow))
			{
				// Begin render pass
				mWindow->beginRendering();

				if (mMonitorController->isRenderingEnabled())
				{
					std::vector<nap::RenderableComponentInstance*> renderableComponents = { mFloorWireFrame.get(), mFloor.get(), mAxesHelpers.get(), mSatellites.get(), mSubs.get(), mSideFills.get() };

					for (auto& entity : mEnvironment->getEntities())
						getRenderableComponentsRecursive(*entity, renderableComponents);

					// Render the world with the right camera directly to screen
					mRenderService->renderObjects(*mWindow, *mCamera, renderableComponents);

					// Render the text overlay
					mTextOverlayController->draw(*mWindow, *mCamera);
				}

				// Render GUI elements
				mGuiService->draw();

				// Stop render pass
				mWindow->endRendering();

				// End recording
				mRenderService->endRecording();
			}
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
