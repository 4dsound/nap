#include "4dsoundapp.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/logger.h>
#include <inputrouter.h>
#include <orthocameracomponent.h>
#include <perspcameracomponent.h>
#include <renderable2dtextcomponent.h>
#include <rendercomponent.h>

#include <imgui/imgui.h>
#include <nsdlgl.h>

// Spatial includes.
#include <Spatial/MultiSpeaker/MultiSpeakerSetup.h>
#include <Spatial/Core/SpatialTypes.h>
#include <Spatial/Monitor/MonitorCameraComponent.h>
#include <Spatial/Core/EnvironmentComponent.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TheWorksApp)
RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
    /**
     * Initialize all the resources and instances used for drawing
     * slowly migrating all functionality to NAP
     */
    bool TheWorksApp::init(utility::ErrorState& error)
    {
        // Retrieve services
        mRenderService	= getCore().getService<nap::RenderService>();
        mSceneService	= getCore().getService<nap::SceneService>();
        mInputService	= getCore().getService<nap::InputService>();
        mGuiService		= getCore().getService<nap::IMGui2Service>();
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

		// Fixing window height when they don't fit the primary screen
		// This has been fixed in NAP 0.3
		{
			SDL_Rect rect;
			SDL_GetDisplayUsableBounds(0, &rect);
			const int max_window_height = rect.h - 50.0f;
			if (mGuiWindow->getHeight() > max_window_height)
				mGuiWindow->setHeight(max_window_height);
			if (mRenderWindow->getHeight() > max_window_height)
				mRenderWindow->setHeight(max_window_height);
		}

        // Get the scene that contains our entities and components
        mScene = mResourceManager->findObject<Scene>("Scene");
        if (!error.check(mScene != nullptr, "unable to find scene with name: %s", "Scene"))
            return false;
        
        // Get the default input router
        mDefaultInputRouter = mScene->findEntity("DefaultInputRouterEntity");
		
		
		// Setup ImGui render windows
		mGuiService->registerWindowForRendering(mGuiWindow);
		mGuiService->registerWindowForRendering(mRenderWindow);

		// Setup ImGui appearance
		ImGui2RenderTarget * gui_render_target = mGuiService->getRenderTargetForWindow(mGuiWindow);
		if (!error.check(gui_render_target != nullptr, "unable to find ImGui2RenderTarget for Gui window"))
			return false;

		ImGui2RenderTarget * monitor_render_target = mGuiService->getRenderTargetForWindow(mRenderWindow);
		if (!error.check(monitor_render_target != nullptr, "unable to find ImGui2RenderTarget for render window"))
			return false;

		mMonitorStyle.setupAppearance(&gui_render_target->getStyle());
		mMonitorStyle.setupAppearance(&monitor_render_target->getStyle());

		auto gui_font = mResourceManager->findObject<Font>("GuiFont");
		if (gui_font != nullptr) {
			gui_render_target->setFont(gui_font);
			monitor_render_target->setFont(gui_font);
		}

        // init monitor gui to load logo resource
        mMonitorGui = std::make_unique<spatial::MonitorGui>();
        mMonitorGui->init(mResourceManager, mSpatialService);
        mMonitorOverlayGui = std::make_unique<spatial::MonitorOverlayGui>();
        mMonitorOverlayGui->init(mResourceManager, *mParameterService);

        // Set the environment script to the command line argument (if any)
        auto globalEntity = mScene->findEntity("global");
        auto environmentComponent = globalEntity->findComponent<spatial::EnvironmentComponentInstance>();
        if (!mCommandLineArgs.empty())
            environmentComponent->setScriptPath(mCommandLineArgs[0]);
        
        // All done!
        return true;
    }
    
    
    // Called when the window is updating
    void TheWorksApp::update(double deltaTime)
    {
        // Use a default input router to forward input events (recursively) to all input components in the default scene
        nap::DefaultInputRouter input_router(true);
        mInputService->processWindowEvents(*mRenderWindow, input_router, { &mScene->getRootEntity() });

		mMonitorOverlayGui->update(*mSpatialService, *mScene);
        mMonitorGui->updateBlinking(deltaTime); // Updates the phase for the blinking of selected entity.
    }
    
    
    // Called when the window is going to render
    void TheWorksApp::render()
    {
        
        

        
        // Destroy old GL context related resources scheduled for destruction
        mRenderService->destroyGLContextResources({ mRenderWindow.get() });
        
        // Prep render window for drawing
        mRenderWindow->makeActive();
        
        // Clear back-buffer
        mRenderService->clearRenderTarget(mRenderWindow->getBackbuffer());
        
        // Find the camera
        auto cameraEntity = mScene->findEntity("camera");
        nap::CameraComponentInstance* camera = nullptr;
        if (cameraEntity->hasComponent<nap::PerspCameraComponentInstance>())
            camera = &cameraEntity->getComponent<nap::PerspCameraComponentInstance>();
        else
            camera = &cameraEntity->getComponent<nap::spatial::MonitorCameraComponentInstance>();
		
		
		// Render monitor
		if (mMonitorGui->getVisibility().showMonitor)
		{
			// Render the world with the right camera directly to screen
			std::vector<RenderableComponentInstance*> renderableComponents;
			mScene->getRootEntity().getComponentsOfTypeRecursive(renderableComponents);
			
			// Fetch 2d renderable components, and subtract them from the list of 3d components, to avoid errors.
			{
				std::vector<Renderable2DTextComponentInstance*> renderable2DTextComponents;
				mScene->getRootEntity().getComponentsOfTypeRecursive(renderable2DTextComponents);
				for (auto comp : renderable2DTextComponents)
				{
					auto i = std::find(renderableComponents.begin(), renderableComponents.end(), comp);
					if (i != renderableComponents.end())
						renderableComponents.erase(i);
				}
			}
			
			// Draw the remaining 3d components.
			mRenderService->renderObjects(mRenderWindow->getBackbuffer(), *camera, renderableComponents);
		}
		
        //////////////// For Marcel's text rendering ////////////////
        // todo : ask Coen how to access these in a less hacky way
        spatial::g_windowSx = mRenderWindow->getWidth();
        spatial::g_windowSy = mRenderWindow->getHeight();
        spatial::g_cameraProjectionMatrix = camera->getProjectionMatrix();
        spatial::g_cameraViewMatrix = camera->getViewMatrix();
		
		//auto orthoCameraEntity = mScene->findEntity("orthoCamera");
        //auto orthoCamera = &orthoCameraEntity->getComponent<nap::OrthoCameraComponentInstance>();
		
		// Draw text overlay.
		auto monitorEntity = mScene->findEntity("monitor");
		if (monitorEntity != nullptr && mMonitorGui->getVisibility().showMonitor)
		{
			Renderable2DTextComponentInstance * textComp = monitorEntity->findComponentByID<Renderable2DTextComponentInstance>("monitorTextOverlayComponent");
			mMonitorGui->drawTextOverlay(*mSpatialService, mRenderWindow.get(), textComp);
		}
		
		{
			mGuiService->makeWindowActive(mRenderWindow);
			mMonitorOverlayGui->draw(*mSpatialService, *mScene, *mMonitorGui, ImVec2(mRenderWindow->getWidth(), mRenderWindow->getHeight()));
			mGuiService->renderWindow(mRenderWindow);
		}
		
        // Swap screen buffers
        mRenderWindow->swap();
		
		
        // Draw our gui
		mRenderService->destroyGLContextResources({ mGuiWindow.get() });
		mGuiWindow->makeActive();
		mRenderService->clearRenderTarget(mGuiWindow->getBackbuffer());
		
		{
			mGuiService->makeWindowActive(mGuiWindow);
			mMonitorGui->update(*mSpatialService, *mScene, glm::vec2(mGuiWindow->getWidth(), mGuiWindow->getHeight()));
			mGuiService->renderWindow(mGuiWindow);
		}
		mGuiWindow->swap();
    }
    
    
    void TheWorksApp::windowMessageReceived(WindowEventPtr windowEvent)
    {
        mRenderService->addEvent(std::move(windowEvent));
    }
    
    
    void TheWorksApp::inputMessageReceived(InputEventPtr inputEvent)
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
					mGuiWindow->makeActive();
				}
				else
				{
					mGuiWindow->hide();
					
					mRenderWindow->show();
					mRenderWindow->makeActive();
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
    
    
    int TheWorksApp::shutdown()
    {
        utility::ErrorState errorState;
        if (!getCore().writeConfigFile(errorState))
            Logger::warn("Failed to write configuration file: %s", errorState.toString().c_str());
        return 0;
    }
    
}
