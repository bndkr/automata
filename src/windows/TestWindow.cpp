#include "automata/Grid.hpp"
#include "TestWindow.hpp"
#include "utils/LoadTextureFromData.hpp"

#include <d3d11.h>

namespace automata
{
	void showTestWindow(bool& show, ID3D11Device* pDevice)
	{
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::Begin("Another Window", &show);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show = false;

		if (ImGui::Button("Create Image"))
		{
			uint32_t height = 200;
			uint32_t width = 200;
			uint32_t bitWidth = 4;
			Grid grid(width, height);

			ID3D11ShaderResourceView* view;
			automata::LoadTextureFromData(grid.getData(), &view, pDevice, width, height);
			ImGui::Image((void*)view, ImVec2(width, height));
		}
		

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}
}
