#pragma once

#include <imgui/imgui.h>

namespace nap
{

    /**
     * Class used to apply hard-coded ImGui style with 4DSOUND color scheme.
     */
    class NAPAPI GuiStyle
    {
    public:
        /**
         * Applies the 4DSOUND ImGui style to the provided ImGuiStyle struct
         * @param style struct containing all ImGui style variables.
         * @param scaling Optionally scale all sizes. Not recommended. Better to use text scaling in Gui::Style resource.
         */
        void apply(ImGuiStyle* style, float scaling = 1.f);

    private:
        void applyColors(ImGuiStyle* style);
    };

}
