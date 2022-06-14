#include "GuiStyle.h"

#include <imgui/imgui.h>

namespace nap
{

    void GuiStyle::apply(ImGuiStyle * style, float scaling)
    {
        style->WindowPadding = ImVec2(0, 0);
        style->WindowRounding = 3.0f * 2;
        style->WindowMinSize = ImVec2(20 * 2, 20 * 2);
        style->FramePadding = ImVec2(2 * 2, 2 * 2);
        style->FrameRounding = 1.f * 2;
        style->WindowBorderSize = 0.f * 2;
        style->ItemSpacing = ImVec2(12 * 2, 6 * 2);
        style->ItemInnerSpacing = ImVec2(4 * 2, 6 * 2);
        style->IndentSpacing = 25.0f * 2;
        style->ScrollbarSize = 15.0f * 2;
        style->ScrollbarRounding = 1.f * 2;
        style->GrabMinSize = 5.f * 2;
        style->GrabRounding = 0.f * 2;
        style->ColumnsMinSpacing = 1.0f * 2;

        applyColors(style);

        style->ScaleAllSizes(scaling);

    }
    
    
    void GuiStyle::applyColors(ImGuiStyle * style)
    {
        assert(style != nullptr);
        
        const ImVec4 GRAY_0(11.f / 255.f, 11.f / 255.f, 12.f / 255.f, 1.f);
        const ImVec4 GRAY_1(22.f / 255.f, 22.f / 255.f, 24.f / 255.f, 1.f);
        const ImVec4 GRAY_2(55.f / 255.f, 55.f / 255.f, 60.f / 255.f, 1.f);
        const ImVec4 GRAY_3(66.f / 255.f, 66.f / 255.f, 72.f / 255.f, 1.f);
        const ImVec4 GRAY_4(88.f / 255.f, 88.f / 255.f, 96.f / 255.f, 1.f);
        const ImVec4 GRAY_5(110.f / 255.f, 110.f / 255.f, 120.f / 255.f, 1.f);
        const ImVec4 GRAY_5_10(110.f / 255.f, 110.f / 255.f, 120.f / 255.f, 0.1f);
        const ImVec4 GRAY_5_15(110.f / 255.f, 110.f / 255.f, 120.f / 255.f, 0.15f);

        const ImVec4 DIM(0.80f, 0.80f, 0.80f, 0.20f);
        const ImVec4 BLUE(.33f, .66f, 1.f, 1.f);
        const ImVec4 NONE(0.f / 255.f, 0.f / 255.f, 0.f / 255.f, 0.f);
        const ImVec4 TEXT(1.f, 1.f, 1.f, 1.f);
        
        // backgrounds
        style->Colors[ImGuiCol_WindowBg]              = GRAY_1;
        style->Colors[ImGuiCol_ChildBg]               = GRAY_1;
        style->Colors[ImGuiCol_PopupBg]               = GRAY_1;
        style->Colors[ImGuiCol_FrameBg]               = GRAY_2;
        style->Colors[ImGuiCol_FrameBgHovered]        = GRAY_3;
        style->Colors[ImGuiCol_FrameBgActive]         = GRAY_4;
        style->Colors[ImGuiCol_TitleBg]               = GRAY_1;
        style->Colors[ImGuiCol_TitleBgActive]         = GRAY_2;
        style->Colors[ImGuiCol_TitleBgCollapsed]      = GRAY_3;
        style->Colors[ImGuiCol_MenuBarBg]             = GRAY_2;
        style->Colors[ImGuiCol_NavWindowingDimBg]     = DIM;
        style->Colors[ImGuiCol_ModalWindowDimBg]      = NONE;

        // texts
        style->Colors[ImGuiCol_Text]                  = TEXT;
        style->Colors[ImGuiCol_TextDisabled]          = GRAY_4;
        style->Colors[ImGuiCol_TextSelectedBg]        = TEXT;

        // borders
        style->Colors[ImGuiCol_Border]                = GRAY_2;
        style->Colors[ImGuiCol_BorderShadow]          = GRAY_5;

        // scrollbar
        style->Colors[ImGuiCol_ScrollbarGrab]         = GRAY_3;
        style->Colors[ImGuiCol_ScrollbarGrabHovered]  = GRAY_4;
        style->Colors[ImGuiCol_ScrollbarGrabActive]   = GRAY_5;
        style->Colors[ImGuiCol_ScrollbarBg]           = NONE;


        style->Colors[ImGuiCol_CheckMark]             = BLUE;
        style->Colors[ImGuiCol_SliderGrab]            = BLUE;
        style->Colors[ImGuiCol_SliderGrabActive]      = ImVec4(1.f, 1.f, 1.f, 0.7f);
        style->Colors[ImGuiCol_Button]                = GRAY_3;
        style->Colors[ImGuiCol_ButtonHovered]         = GRAY_4;
        style->Colors[ImGuiCol_ButtonActive]          = GRAY_5;
        
        // header
        style->Colors[ImGuiCol_Header]                = NONE;
        style->Colors[ImGuiCol_HeaderHovered]         = GRAY_5_10;
        style->Colors[ImGuiCol_HeaderActive]          = GRAY_5_15;
        
        // separator
        style->Colors[ImGuiCol_Separator]             = GRAY_3;
        style->Colors[ImGuiCol_SeparatorHovered]      = GRAY_3;
        style->Colors[ImGuiCol_SeparatorActive]       = GRAY_3;
        
        // resize grip
        style->Colors[ImGuiCol_ResizeGrip]            = NONE;
        style->Colors[ImGuiCol_ResizeGripHovered]     = GRAY_2;
        style->Colors[ImGuiCol_ResizeGripActive]      = BLUE;
        
        // tab
//            style->Colors[ImGuiCol_Tab]                   = ImVec4(0.08f, 0.08f, 0.09f, 0.83f);
//            style->Colors[ImGuiCol_TabHovered]            = ImVec4(0.33f, 0.34f, 0.36f, 0.83f);
        
        style->Colors[ImGuiCol_Tab]                   = GRAY_2;
        style->Colors[ImGuiCol_TabHovered]            = GRAY_4;
        style->Colors[ImGuiCol_TabActive]             = GRAY_3;
        style->Colors[ImGuiCol_TabUnfocused]          = GRAY_2;
        style->Colors[ImGuiCol_TabUnfocusedActive]    = GRAY_3;
        
        // plots
        style->Colors[ImGuiCol_PlotLines]             = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        style->Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        style->Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        style->Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        
        // misc
        style->Colors[ImGuiCol_DragDropTarget]        = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
        style->Colors[ImGuiCol_NavHighlight]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style->Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        
    }

}
