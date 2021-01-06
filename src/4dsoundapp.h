#pragma once

// Spatial includes
#include <Spatial/Monitor/TextOverlayController.h>

// Gui Layout includes
#include <Gui/Gui.h>

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
		ResourceManager*		mResourceManager = nullptr;		///< Manages all the loaded data
		std::string				mFileName = "app_structure.json"; ///< The JSON file that is loaded on initialization
		RenderService*			mRenderService = nullptr;		///< Render Service that handles render calls
		SceneService*			mSceneService = nullptr;		///< Manages all the objects in the scene
		InputService*			mInputService = nullptr;		///< Input service for processing input
		IMGuiService*			mGuiService = nullptr;			///< Manages GUI related update / draw calls
		ParameterService*       mParameterService = nullptr;
		ResourcePtr<RenderWindow> mGuiWindow;						///< Pointer to the gui window
		bool                    mGuiWindowIsVisible = true;     ///< Is the GUI window visible?
		ResourcePtr<RenderWindow> mRenderWindow;					///< Pointer to the render window
		ObjectPtr<Scene>		mScene = nullptr;				///< Pointer to the main scene
		ObjectPtr<spatial::TextOverlayControllerInstance> mTextOverlayController = nullptr;
		ObjectPtr<PerspCameraComponentInstance> mCamera = nullptr; ///< The monitor camera
		ObjectPtr<gui::Gui> mGui = nullptr;
		ObjectPtr<gui::Gui> mMonitorGui = nullptr;

        spatial::SpatialService* mSpatialService = nullptr;  ///< Spatial sound service
        std::vector<std::string> mCommandLineArgs;				///< List with command line arguments

		bool mCtrlKeyPressed = false; // Indicates wether the ctrl key is pressed
	};
}
