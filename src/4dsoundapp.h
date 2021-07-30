#pragma once

// Spatial includes
#include <Spatial/Monitor/TextOverlayController.h>
#include <Spatial/Monitor/MonitorController.h>
#include <Spatial/Core/EnvironmentComponent.h>
#include <Spatial/Core/StateMachine.h>

// Gui Layout includes
#include <Gui/Gui.h>
#include <Gui/Window.h>

// Core includes
#include <nap/resourcemanager.h>
#include <nap/resourceptr.h>

// Module includes
#include <renderservice.h>
#include <imguiservice.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <parameter.h>
#include <parameterservice.h>
#include <Spatial/Core/SpatialService.h>
#include <scene.h>
#include <renderwindow.h>
#include <perspcameracomponent.h>
#include <entity.h>
#include <rendercomponent.h>
#include <app.h>
#include <rendervideocomponent.h>

namespace nap
{
	using namespace rtti;

	/**
	 * Main application that is called from within the main loop
	 */
	class SpatialSoundApp : public App
	{
		RTTI_ENABLE(App)
	public:
		/**
		 * Constructor
		 * @param core instance of the NAP core system
		 */
		SpatialSoundApp(nap::Core& core) : App(core) { }
		
		/**
		 * Initialize all the services and app specific data structures
		 * @param error contains the error code when initialization fails
		 * @return if initialization succeeded
		*/
		bool init(utility::ErrorState& error) override;
		
		/**
		 * Update is called every frame, before render.
		 * @param deltaTime the time in seconds between calls
		 */
		void update(double deltaTime) override;

		/**
		 * Render is called after update. Use this call to render objects to a specific target
		 */
		void render() override;

		/**
		 * Called when the app receives a window message.
		 * @param windowEvent the window message that occurred
		 */
		void windowMessageReceived(WindowEventPtr windowEvent) override;
		
		/**
		 * Called when the app receives an input message (from a mouse, keyboard etc.)
		 * @param inputEvent the input event that occurred
		 */
		void inputMessageReceived(InputEventPtr inputEvent) override;
		
		/**
		 * Called when the app is shutting down after quit() has been invoked
		 * @return the application exit code, this is returned when the main loop is exited
		 */
		virtual int shutdown() override;

		/**
		 * USed by the main function to pass the command line arguments.
		 * @param count number of arguments
		 * @param args arguments
		 */
		void setCommandLineArguments(int count, char* args[])
		{
		    for (auto i = 1; i < count; ++i)
		        mCommandLineArgs.emplace_back(args[i]);
		}

	private:
		ResourceManager* mResourceManager = nullptr;	// Manages all the loaded data
		RenderService* mRenderService = nullptr;		// Render Service that handles render calls
		SceneService* mSceneService = nullptr;			// Manages all the objects in the scene
		InputService* mInputService = nullptr;			// Input service for processing input
		IMGuiService* mGuiService = nullptr;			// Manages GUI related update / draw calls

		ResourcePtr<RenderWindow> mSecondaryWindow;		// Pointer to the detached gui window
		ResourcePtr<RenderWindow> mWindow;				// Pointer to the render window
		ObjectPtr<Scene> mScene = nullptr;				// Pointer to the main scene
		ResourcePtr<StateMachine> mEnvironmentStateMachine = nullptr; // Pointer to the environment state
		ResourcePtr<StateMachine::State> mEnvironmentStartupState = nullptr; // State indicating the environment is starting up

		ObjectPtr<PerspCameraComponentInstance> mCamera = nullptr;
		ObjectPtr<RenderableComponentInstance> mFloorWireFrame = nullptr;
		ObjectPtr<RenderableComponentInstance> mFloor = nullptr;
		ObjectPtr<RenderableComponentInstance> mAxesHelpers = nullptr;
		ObjectPtr<RenderableComponentInstance> mSatellites = nullptr;
        ObjectPtr<RenderableComponentInstance> mSubs = nullptr;
        ObjectPtr<RenderableComponentInstance> mSideFills = nullptr;
		ObjectPtr<spatial::EnvironmentComponentInstance> mEnvironment = nullptr;
		ObjectPtr<spatial::TextOverlayControllerInstance> mTextOverlayController = nullptr;
		ObjectPtr<gui::GuiWindow> mGuiWindow = nullptr;
		ObjectPtr<gui::GuiWindow> mDetachedGuiWindow = nullptr;
		ObjectPtr<gui::Gui> mSplashScreenGui = nullptr;
		ObjectPtr<gui::Gui> mMonitorGui = nullptr;
		ObjectPtr<spatial::MonitorController> mMonitorController = nullptr;
		ObjectPtr<RenderWindow> mStartupWindow = nullptr;

		bool mSecondaryWindowVisible = false;     		// Is the GUI window currently visible?
		bool mPrimaryWindowVisible = false;			// Is the main window visible?
		bool mStartupWindowVisible = true;			// Is the startup window visible?
		bool mCtrlKeyPressed = false; 				// Indicates wether the ctrl key is pressed

		std::string mFileName = "app_structure.json"; 	// The JSON file that is loaded on initialization
        std::vector<std::string> mCommandLineArgs;		// List with command line arguments
	};
}
