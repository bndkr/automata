#include "Conways.hpp"

#include "imgui/imgui.h"
#include "utils/LoadTextureFromData.hpp"

#include <d3d11.h>
#include <random>

Conways::Conways(uint64_t height, uint64_t width, uint32_t scale, ID3D11Device* pDevice)
  : m_height(height),
    m_width(width),
    m_grid(width, height),
    m_upsampledGrid(width * scale, height * scale),
    m_scale(scale),
    m_pDevice(pDevice),
    m_texture(NULL),
    m_view(NULL)
{

}

void Conways::showAutomataWindow()
{


  ImGui::Image((void*)m_view, ImVec2(m_grid.getWidth() * m_scale,
                                     m_grid.getHeight() * m_scale));
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}

void Conways::displayGrid()
{

}

void Conways::updateGrid()
{

}

bool Conways::checkCell(uint32_t row, uint32_t col, bool wrap)
{

}