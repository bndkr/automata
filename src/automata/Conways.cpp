#include "Conways.hpp"

#include "imgui/imgui.h"
#include "utils/LoadTextureFromData.hpp"

#include <d3d11.h>
#include <random>
#include <string>

Conways::Conways(uint64_t height, uint64_t width, uint32_t scale,
                 ID3D11Device* pDevice)
  : m_height(height),
    m_width(width),
    m_grid(width, height),
    m_upsampledGrid(width * scale, height * scale),
    m_rule(std::set<uint8_t>{3}, std::set<uint8_t>{2, 3}),
    m_defaultRule(std::set<uint8_t>{3}, std::set<uint8_t>{2, 3}),
    m_scale(scale),
    m_neighborhoodSize(8),
    m_presetRules(),
    m_wrap(true),
    m_pDevice(pDevice),
    m_texture(NULL),
    m_view(NULL)
{
  loadGrid();

  m_presetRules.insert({"M1 Conway's game of life",
                        Rule{std::set<uint8_t>{3}, std::set<uint8_t>{2, 3}}});
  m_presetRules.insert({"M1 Islands", Rule{std::set<uint8_t>{5, 6, 7, 8},
                                           std::set<uint8_t>{4, 5, 6, 7, 8}}});
  m_presetRules.insert(
    {"M2 Life",
     Rule{std::set<uint8_t>{8, 9, 10, 11, 12, 13},
          std::set<uint8_t>{6, 7, 8, 9, 10, 11}}}); // has a diagonal glider
  m_presetRules.insert(
    {"M2 fireworks",
     Rule{std::set<uint8_t>{4},
          std::set<uint8_t>{11, 12, 13, 14}}}); // has a orthoganal glider
  // little flaming fireballs that can spawn others
  m_presetRules.insert(
    {"M2 flames", Rule{std::set<uint8_t>{4},
                       std::set<uint8_t>{8, 9, 10, 11, 12, 13, 14, 15}}});
  m_presetRules.insert(
    {"M2 little roads",
     Rule{std::set<uint8_t>{2, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24},
          std::set<uint8_t>{12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
                            24}}}); // little roads
  m_presetRules.insert(
    {"Weighted flames", Rule{std::set<uint8_t>{3, 13, 14, 15, 16},
                             std::set<uint8_t>{6, 7, 8, 9, 10, 11, 12, 13, 14,
                                               15, 16}}}); // more flames
}

void Conways::showAutomataWindow()
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

  if (ImGui::Button("Clear"))
  {
    m_grid.clear();
    m_upsampledGrid.clear();
    loadGrid();
  }
  ImGui::SameLine();
  if (!running && ImGui::Button("Start"))
  {
   resetGrid();
   running = true;
  }
  ImGui::SameLine();
  if (running && ImGui::Button("Stop"))
  {
    running = false;
  }
  ImGui::SameLine();
  if (!running && ImGui::Button("Step"))
    updateGrid();
  if (!running)
  {
    ImGui::SameLine();
    if (ImGui::Button("Resume"))
    {
      updateGrid();
      running = true;
    }
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
  
}

void Conways::showRuleMenu(bool& show)
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
  if (ImGui::Button("Randomize Rule"))
  {
    m_rule.m_birthConditions.clear();
    m_rule.m_surviveConditions.clear();
    for (uint32_t i = 0; i <= m_neighborhoodSize; i++)
    {
      if (rand() % 3 == 0)
      {
        m_rule.m_birthConditions.insert(i);
      }
      if (rand() % 3 == 0)
      {
        m_rule.m_surviveConditions.insert(i);
      }
    }
  }
  if (ImGui::Button("Clear rule"))
  {
    m_rule = Rule(std::set<uint8_t>{}, std::set<uint8_t>{});
  }
  const char* items[] = {"M1 Conway's game of life",
                         "M1 Islands",
                         "M2 Life",
                         "M2 fireworks",
                         "M2 flames",
                         "M2 little roads",
                         "Weighted flames"};
  static int item_current_idx = 0;
  const char* combo_preview_value = items[item_current_idx];
  if (ImGui::BeginCombo("Load Preset Rule", combo_preview_value))
  {
    for (int n = 0; n < IM_ARRAYSIZE(items); n++)
    {
      const bool is_selected = (item_current_idx == n);
      if (ImGui::Selectable(items[n], is_selected))
      {
        item_current_idx = n;
        for (const auto& rule : m_presetRules)
        {
          if (rule.first == items[item_current_idx])
          {
            m_rule = rule.second;
          }
        }
      }
      if (is_selected)
        ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
  ImGui::Text("Click to toggle rules");
  ImGui::Text("Birth Conditions:");
  for (uint8_t i = 0; i <= m_neighborhoodSize; i++)
  {
    if (m_rule.m_birthConditions.count(i))
      ImGui::PushStyleColor(ImGuiCol_Button,
                            (ImVec4)ImColor::HSV(0.4f, 0.6f, 0.6f));
    else
      ImGui::PushStyleColor(ImGuiCol_Button,
                            (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
    if (ImGui::Button((std::string("b") + std::to_string(i)).c_str()))
    {
      if (m_rule.m_birthConditions.count(i))
        m_rule.m_birthConditions.erase(i);
      else
        m_rule.m_birthConditions.insert(i);
    }
    ImGui::PopStyleColor();
    if (i != m_neighborhoodSize)
      ImGui::SameLine();
  }
  ImGui::Text("Survival Conditions:");
  for (uint8_t i = 0; i <= m_neighborhoodSize; i++)
  {
    if (m_rule.m_surviveConditions.count(i))
      ImGui::PushStyleColor(ImGuiCol_Button,
                            (ImVec4)ImColor::HSV(0.4f, 0.6f, 0.6f));
    else
      ImGui::PushStyleColor(ImGuiCol_Button,
                            (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
    if (ImGui::Button((std::string("s") + std::to_string(i)).c_str()))
    {
      if (m_rule.m_surviveConditions.count(i))
        m_rule.m_surviveConditions.erase(i);
      else
        m_rule.m_surviveConditions.insert(i);
    }
    ImGui::PopStyleColor();
    if (i != m_neighborhoodSize)
      ImGui::SameLine();
  }

  std::string ruleStr("Current Rule: {Birth: ");
  for (const auto& rule : m_rule.m_birthConditions)
  {
    ruleStr += std::to_string(rule) + " ";
  }
  ruleStr += ", Survival: ";
  for (const auto& rule : m_rule.m_surviveConditions)
  {
    ruleStr += std::to_string(rule) + " ";
  }
  ruleStr += "}";
  ImGui::Text(ruleStr.c_str());
  ImGui::End();
}

void Conways::loadGrid()
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
        if (!m_rule.survived(aliveNeighbors))
        {
          m_grid.setCell(h, w, dead);
        }
      }
      else
      { // is the cell is dead
        if (m_rule.born(aliveNeighbors))
        {
          m_grid.setCell(h, w, alive);
        }
      }
    }
  }
  m_grid.applyChanges();
  upsampleGrid(m_grid, m_upsampledGrid, m_scale);
  loadGrid();
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
  loadGrid();
}

uint32_t Conways::countNeighbors(int32_t height, int32_t width)
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

bool Conways::checkCell(int32_t row, int32_t col)
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
  return m_grid.checkCell(r, c);
}
