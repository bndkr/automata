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
    m_wrap(false),
    m_pDevice(pDevice),
    m_texture(NULL),
    m_view(NULL)
{
  displayGrid();
}

void Conways::showAutomataWindow()
{
  ImGui::Checkbox("Wrap edges", &m_wrap);

  if (ImGui::Button("Start"))
  {
    resetGrid();
  }

  // if (ImGui::Button("Set Simulation"))
  // {
  //   updateGrid();
  // }

  static int timer = 0;
  static int timerReset = 0;
  ImGui::SliderInt("Simulation Speed", &timerReset, 0, 60);
  if (timer > timerReset)
  {
    updateGrid();
    timer = 0;
  }
  timer++;

  ImGui::Image((void*)m_view, ImVec2(m_grid.getWidth() * m_scale,
                                     m_grid.getHeight() * m_scale));
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}

void Conways::displayGrid()
{
  if (m_texture)
    m_texture->Release();
  if (m_view)
    m_view->Release();

  automata::LoadTextureFromData(m_upsampledGrid.getData(), &m_view, &m_texture,
                                m_pDevice, m_width * m_scale,
                                m_height * m_scale);
}

void Conways::updateGrid()
{
  Color dead = {0, 0, 0, 0};
  Color alive = {255, 255, 255, 255};

  for (uint32_t h = 0; h < m_height; h++)
  {
    for (uint32_t w = 0; w < m_width; w++)
    {
      uint32_t aliveNeighbors = countNeighbors(h, w);
      if (m_grid.checkCell(h, w))
      { // if the cell is alive
        if (aliveNeighbors < 2 || aliveNeighbors > 3)
        { // and has 2 or 3 living neighbors
          m_grid.setCell(h, w, dead);
        }
      }
      else
      { // is the cell is dead
        if (aliveNeighbors == 3)
        { // and has exactly 3 living neighbors
          m_grid.setCell(h, w, alive);
        }
      }
    }
  }
  m_grid.applyChanges();
  upsampleGrid(m_grid, m_upsampledGrid, m_scale);
  displayGrid();
}

void Conways::resetGrid()
{
  Color white{255, 255, 255, 255};
  m_grid.clear();
  m_upsampledGrid.clear();
  for (uint32_t h = 0; h < m_height; h++)
  {
    for (uint32_t w = 0; w < m_width; w++)
    {
      if (rand() % 2 == 0)
      {
        m_grid.setCell(h, w, white);
      }
    }
  }
  m_grid.applyChanges();
  upsampleGrid(m_grid, m_upsampledGrid, m_scale);
  displayGrid();
}

uint32_t Conways::countNeighbors(int32_t height, int32_t width)
{
  return checkCell(height - 1, width - 1) + checkCell(height - 1, width) +
         checkCell(height - 1, width + 1) + checkCell(height, width - 1) +
         checkCell(height, width + 1) + checkCell(height + 1, width - 1) +
         checkCell(height + 1, width) + checkCell(height + 1, width + 1);

}

bool Conways::checkCell(int32_t row, int32_t col)
{
  int32_t r = row;
  int32_t c = col;
  if (m_wrap)
  {
    if (row == -1)
      r = m_height - 1;
    if (row == m_height)
      r = 0;
    if (col == -1)
      c = m_width - 1;
    if (col == m_width)
      c = 0;
  }
  else
  {
    if (row == -1 || row == m_height || col == -1 || col == m_width)
      return false;
  }

  return m_grid.checkCell(r, c);
}