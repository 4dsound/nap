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
#include <Spatial/Gui/ImGuiExtensions.h>
#include <audio/utility/audiofunctions.h>

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
			if (comp->get_type() != RTTI_OF(Renderable2DTextComponentInstance))
				components.emplace_back(comp);
		for (auto& entity : entity.getChildren())
			getRenderableComponentsRecursive(*entity, components);
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
        mSpatialService = getCore().getService<spatial::SpatialService>();
        mParameterService = getCore().getService<nap::ParameterService>();
        
        // Fetch the resource manager
        mResourceManager = getCore().getResourceManager();
        
        // Fetch the scene
        mScene = mResourceManager->findObject<Scene>("Scene");
        
        // load resources from file
        if (!mResourceManager->loadFile(mFileName, error))
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
		if (!error.check(cameraEntity != nullptr, "unable to find camera"))
			return false;
		mCamera = cameraEntity->findComponent<nap::PerspCameraComponentInstance>();
		if (!error.check(mCamera != nullptr, "unable to find camera"))
			return false;

		auto textOverlayEntity = mScene->findEntity("MonitorTextOverlay");
		if (!error.check(textOverlayEntity != nullptr, "unable to find monitor text overlay"))
			return false;
		mTextOverlayController = textOverlayEntity->findComponent<nap::spatial::TextOverlayControllerInstance>();
		if (!error.check(mTextOverlayController != nullptr, "unable to find text overlay"))
			return false;

		auto renderableTextComponent = textOverlayEntity->findComponent<Renderable2DTextComponentInstance>();
		if (!error.check(renderableTextComponent != nullptr, "unable to find text overlay"))
			return false;

        // Set the environment script to the command line argument (if any)
        auto globalEntity = mScene->findEntity("global");
        auto environmentComponent = globalEntity->findComponent<spatial::EnvironmentComponentInstance>();
        if (!mCommandLineArgs.empty())
            environmentComponent->setScriptPath(mCommandLineArgs[0]);

		auto multiSpeakerSetup = mResourceManager->findObject<spatial::MultiSpeakerSetup>("multiSetup");
		mAudioDeviceSettingsGui = std::make_unique<audio::AudioDeviceSettingsGui>(mSpatialService->getAudioService());
		mMultiSpeakerSetupGui = std::make_unique<spatial::MultiSpeakerSetupGui>(multiSpeakerSetup.get());

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
		ImGui::Begin("Info");
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		ImGui::End();

		mGuiService->selectWindow(mGuiWindow);
		mAudioDeviceSettingsGui->drawGui();

		ImGui::NewLine();

		// Input VU meters.
		if (mSpatialService->getAudioService().isOpened())
		{
			int fullRowCount = int(mSpatialService->getInputChannelCount() / 16);
			int channelOffset = 0;
			for (auto row = 0; row <= fullRowCount; ++row)
			{
				for (auto channel = 0; channel < (row == fullRowCount ? mSpatialService->getInputChannelCount() % 16 : 16); ++channel)
				{
					auto label = std::to_string(channelOffset + channel + 1);
					// draw level meter
					auto dbLevel = audio::toDB(mSpatialService->getInputLevel(channelOffset + channel));
					if (channel > 0) ImGui::SameLine();
					ImGui::VUMeter(ImVec2(15.f, 50.f), dbLevel, -48.f, 0.f, false, false, 0.f, label);
				}
				channelOffset += 16;
			}
		}

		ImGui::NewLine();

		mMultiSpeakerSetupGui->draw();
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

			std::vector<nap::RenderableComponentInstance*> renderableComponents;
			getRenderableComponentsRecursive(mScene->getRootEntity(), renderableComponents);

			// Render the world with the right camera directly to screen
			mRenderService->renderObjects(*mRenderWindow, *mCamera, renderableComponents);

			// Render the text overlay
			mTextOverlayController->draw(*mRenderWindow, *mCamera);

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
