#pragma once

// internal includes
#include "sequenceeditor.h"
#include "sequencetrackview.h"
#include "sequenceeditorguistate.h"
#include "sequenceeditorguiactions.h"

// external includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <rtti/objectptr.h>
#include <imgui/imgui.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequenceEditorGUIView;
	class SequenceEditorView;
	class SequenceTrackView;

	/**
	 * SequenceEditorGUI
	 * A GUI resource that can be instantiated to draw a GUI (view) for the sequence editor
	 */
	class NAPAPI SequenceEditorGUI : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		 * @param errorState contains any errors
		 * @return true on success
		 */
		virtual bool init(utility::ErrorState& errorState);

		/**
		 * called before deconstruction
		 */
		virtual void onDestroy();

		/**
		 * Call this method to draw the GUI
		 */
		void show();
	public:
		// properties
		ResourcePtr<SequenceEditor> mSequenceEditor = nullptr; ///< Property: 'Sequence Editor' link to editor resource
	protected:
		// instantiated view
		std::unique_ptr<SequenceEditorGUIView> mView = nullptr; 
	};

	/**
	 * SequenceEditorGUIView
	 * Responsible for drawing the GUI for the sequence editor
	 * Needs reference to controller
	 */
	class NAPAPI SequenceEditorGUIView
	{
		friend class SequenceTrackView;
	public:
		/**
		 * Constructor
		 * @param editor reference to editor
		 * @param id id of the GUI resource, used to push ID by IMGUI
		 */
		SequenceEditorGUIView(SequenceEditor& editor, std::string id);

		/**
		 * shows the editor interface
		 */
		void show();

		/**
		 * static method for registering a view type that draws the appropriate track type
		 */
		static bool registerTrackViewType(rttr::type trackType, rttr::type viewType);
	private:
		/**
		 * Draws the tracks of the sequence
		 * @param sequencePlayer reference to sequenceplayer
		 * @param sequence reference to sequence
		 */
		void drawTracks(const SequencePlayer& sequencePlayer, const Sequence &sequence);

		/**
		 * drawPlayerController
		 * draws player controller bar
		 * @param player reference to player
		 */
		void drawPlayerController(SequencePlayer& player);

		/**
		 * drawTimelinePlayerPosition
		 * draws line of player position
		 * @param sequence reference to sequence
		 * @param player reference to player
		 */
		void drawTimelinePlayerPosition(const Sequence& sequence, SequencePlayer& player);

		/**
		 * Handles insertion of track popup
		 */
		void handleInsertTrackPopup();

		/**
		 * handleLoadPopup
		 * handles load popup
		 */
		void handleLoadPopup();

		/**
		 * handleSaveAsPopup
		 * handles save as popup
		 */
		void handleSaveAsPopup();
	protected:
		// reference to editor
		SequenceEditor& mEditor;

		// holds current state information
		SequenceEditorGUIState mState;

		// id
		std::string mID;

		// map of all track views
		std::unordered_map<rttr::type, std::unique_ptr<SequenceTrackView>> mViews;
	};
}
