#include "Gradient.hpp"

#include "imgui/imgui.h"
#include "utils/LoadTextureFromData.hpp"

#include <d3d11.h>
#include <random>
#include <string>

Gradient::Gradient(uint64_t height, uint64_t width, uint32_t scale,
                 ID3D11Device* pDevice)
  : m_height(height),
    m_width(width),
    m_grid(width, height),
    m_upsampledGrid(width * scale, height * scale),
    m_rule(std::make_pair(510, 765), std::make_pair(255, 765)),
    m_defaultRule(std::make_pair(510, 765), std::make_pair(255, 765)),
    m_scale(scale),
    m_neighborhoodSize(8),
    m_presetRules(),
    m_wrap(true),
    m_pDevice(pDevice),
    m_texture(NULL),
    m_view(NULL)
{
  loadGrid();
}

void Gradient::showAutomataWindow()
{
  static int timer = 0;
  static int timerReset = 0;
  static bool displayRuleMenu = false;
  static bool running = false;
  static bool drawClick = false;

  if (ImGui::Button("Show Rule Editor"))
    displayRuleMenu = true;

  if (displayRuleMenu)
    showRuleMenu(displayRuleMenu);

  ImGui::Checkbox("Wrap edges", &m_wrap);

  if (running)
  {
    if (ImGui::Button("Stop"))
      running = false;
    ImGui::SameLine();
    if (ImGui::Button("Reset"))
      resetGrid();
  }
  else
  {
    if (ImGui::Button("Start"))
    {
      resetGrid();
      running = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Step"))
      updateGrid();
    ImGui::SameLine();
    if (ImGui::Button("Resume"))
      running = true;
  }
 
  ImGui::SliderInt("Simulation Speed", &timerReset, 0, 60);
  if (timer > timerReset && running)
  {
    updateGrid();
    timer = 0;
  }
  if (running)
    timer++;

   ImGui::Image((void*)m_view, ImVec2(m_grid.getWidth() * m_scale,
                                      m_grid.getHeight() * m_scale));
  /*
  bool isHovered = ImGui::IsItemHovered();
  ImVec2 mousePositionAbsolute = ImGui::GetMousePos();
  ImVec2 screenPositionAbsolute = ImGui::GetItemRectMin();
  ImVec2 mousePositionRelative =
    ImVec2(mousePositionAbsolute.x - screenPositionAbsolute.x,
           mousePositionAbsolute.y - screenPositionAbsolute.y);
  if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && isHovered &&
      mousePositionRelative.x < m_scale * m_width &&
      mousePositionRelative.x >= 0 &&
      mousePositionRelative.y < m_scale * m_height &&
      mousePositionRelative.y >= 0)
  { // did the user click on the grid?
    m_grid.setCell(mousePositionRelative.y / m_scale,
                   mousePositionRelative.x / m_scale,
                   Color{255, 255, 255, 255});
    m_grid.applyChanges();
    upsampleGrid(m_grid, m_upsampledGrid, m_scale);
    loadGrid();
  }
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  draw_list->AddImage(m_view, ImVec2(1, 100), ImVec2(1, 100));
  */
}

void Gradient::showRuleMenu(bool& show)
{
  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoResize;
  ImGui::Begin("Rule Editor", &show, flags);

  static int neighborhoodSelected = 0;
  if (ImGui::Combo("Neighborhood Size", &neighborhoodSelected,
                   "Moore distance 1\0"
                   "Moore Distance 2\0"
                   "Von Neumann 1\0"
                   "Von Neumann 2\0"
                   "Weighted\0\0"))
  {
    switch (neighborhoodSelected)
    {
    case 0:
      m_neighborhoodSize = 8;
      break;
    case 1:
      m_neighborhoodSize = 24;
      break;
    case 2:
      m_neighborhoodSize = 4;
      break;
    case 3:
      m_neighborhoodSize = 12;
      break;
    case 4:
      m_neighborhoodSize = 16;
    }
  }

  if (ImGui::Button("Reset rule to default"))
    m_rule = m_defaultRule;

  ImGui::DragIntRange2("Birth", &m_rule.m_birthConditions.first,
                       &m_rule.m_birthConditions.second, 1.0f, 0,
                       255 * m_neighborhoodSize);
  ImGui::DragIntRange2("Survive", &m_rule.m_surviveConditions.first,
                       &m_rule.m_surviveConditions.second, 1.0f, 0,
                       255 * m_neighborhoodSize);
  ImGui::Text("Valid range: %d to %d", 0, 255 * m_neighborhoodSize);

  ImGui::End();
}

void Gradient::loadGrid()
{
  if (m_texture)
    m_texture->Release();
  if (m_view)
    m_view->Release();

  automata::LoadTextureFromData(m_upsampledGrid.getData(), &m_view, &m_texture,
                                m_pDevice, m_width * m_scale,
                                m_height * m_scale);
}

void Gradient::updateGrid()
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
        if (!m_rule.survived(aliveNeighbors))
        {
          m_grid.setCell(h, w, dead);
        }
      }
      else
      { // is the cell is dead
        if (m_rule.born(aliveNeighbors))
        {
          uint8_t g = rand() % 255;
          m_grid.setCell(h, w, Color{g, g, g, 255});
        }
      }
    }
  }
  m_grid.applyChanges();
  upsampleGrid(m_grid, m_upsampledGrid, m_scale);
  loadGrid();
}

void Gradient::resetGrid()
{
  Color white{255, 255, 255, 255};
  m_grid.clear();
  m_upsampledGrid.clear();
  for (uint32_t h = 0; h < m_height; h++)
  {
    for (uint32_t w = 0; w < m_width; w++)
    {
      uint8_t g = rand() % 255;
      m_grid.setCell(h, w, Color{g, g, g, 255});
    }
  }
  m_grid.applyChanges();
  upsampleGrid(m_grid, m_upsampledGrid, m_scale);
  loadGrid();
}

uint32_t Gradient::countNeighbors(int32_t height, int32_t width)
{
  if (m_neighborhoodSize == 8)
    return checkCell(height - 1, width - 1) + checkCell(height - 1, width) +
           checkCell(height - 1, width + 1) + checkCell(height, width - 1) +
           checkCell(height, width + 1) + checkCell(height + 1, width - 1) +
           checkCell(height + 1, width) + checkCell(height + 1, width + 1);
  else if (m_neighborhoodSize == 24)
    return checkCell(height - 1, width - 1) + checkCell(height - 1, width) +
           checkCell(height - 1, width + 1) + checkCell(height, width - 1) +
           checkCell(height, width + 1) + checkCell(height + 1, width - 1) +
           checkCell(height + 1, width) + checkCell(height + 1, width + 1) +
           checkCell(height - 2, width - 2) + checkCell(height - 2, width - 1) +
           checkCell(height - 2, width) + checkCell(height - 2, width + 1) +
           checkCell(height - 2, width + 2) + checkCell(height - 1, width - 2) +
           checkCell(height - 1, width + 2) + checkCell(height, width - 2) +
           checkCell(height, width + 2) + checkCell(height + 1, width - 2) +
           checkCell(height + 1, width + 2) + checkCell(height + 2, width - 2) +
           checkCell(height + 2, width - 1) + checkCell(height + 2, width) +
           checkCell(height + 2, width + 1) + checkCell(height + 2, width + 2);
  else if (m_neighborhoodSize == 4)
    return checkCell(height - 1, width) + checkCell(height, width - 1) +
           checkCell(height + 1, width) + checkCell(height, width + 1);
  else if (m_neighborhoodSize == 12)
    return checkCell(height - 2, width) + checkCell(height - 1, width - 1) +
           checkCell(height - 1, width) + checkCell(height - 1, width + 1) +
           checkCell(height, width - 2) + checkCell(height, width - 1) +
           checkCell(height, width + 1) + checkCell(height, width + 2) +
           checkCell(height + 1, width - 1) + checkCell(height + 1, width) +
           checkCell(height + 1, width + 1) + checkCell(height + 2, width);
  else if (m_neighborhoodSize == 16) // inner 8 cells have twice the weight
    return (2 * checkCell(height - 1, width - 1) +
            2 * checkCell(height - 1, width) +
            2 * checkCell(height - 1, width + 1) +
            2 * checkCell(height, width - 1) +
            2 * checkCell(height, width + 1) +
            2 * checkCell(height + 1, width - 1) +
            2 * checkCell(height + 1, width) +
            2 * checkCell(height + 1, width + 1) +
            checkCell(height - 2, width - 2) +
            checkCell(height - 2, width - 1) + checkCell(height - 2, width) +
            checkCell(height - 2, width + 1) +
            checkCell(height - 2, width + 2) +
            checkCell(height - 1, width - 2) +
            checkCell(height - 1, width + 2) + checkCell(height, width - 2) +
            checkCell(height, width + 2) + checkCell(height + 1, width - 2) +
            checkCell(height + 1, width + 2) +
            checkCell(height + 2, width - 2) +
            checkCell(height + 2, width - 1) + checkCell(height + 2, width) +
            checkCell(height + 2, width + 1) +
            checkCell(height + 2, width + 2)) /
           2;
  else
    return 0;
}

uint32_t Gradient::checkCell(int32_t row, int32_t col)
{
  int32_t r = row;
  int32_t c = col;
  if (m_wrap)
  {
    if (row <= -1)
      r = m_height + row;
    if (row >= m_height)
      r = row - m_height;
    if (col <= -1)
      c = m_width + col;
    if (col >= m_width)
      c = col - m_width;
  }
  else
  {
    if (row < 0 || row >= m_height || col < 0 || col >= m_width)
      return false;
  }
  auto color = m_grid.getCell(r, c);
  // return the average of rgb values
  return ((uint32_t)color.r + color.b + color.g) / 3;
}
