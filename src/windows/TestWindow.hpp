#ifndef WINDOWS_TEST_WINDOW
#define WINDOWS_TEST_WINDOW

#include "imgui/imgui.h"
#include <d3d11.h>

namespace automata
{
	void showTestWindow(bool& show, ID3D11Device* pDevice);
}

#endif