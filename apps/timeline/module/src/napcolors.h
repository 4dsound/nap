#pragma once

#include <imgui/imgui.h>

namespace nap
{
	namespace guicolors
	{
		const ImU32 red = 4285098440;
		const ImU32 black = 4280685585;
		const ImU32 white = 4288711819;
		const ImU32 lightGrey = 4285750877;
		const ImU32 darkGrey = 4285158482;

		const ImU32 curvecolors[4] =
		{
			red,
			ImGui::ColorConvertFloat4ToU32(ImVec4(0, 1, 0, 1)),
			ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 1, 1)),
			ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 0, 1))
		};
	}
}
