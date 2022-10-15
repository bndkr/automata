#include "Conways.hpp"

#include "imgui/imgui.h"
#include "utils/LoadTextureFromData.hpp"

#include <d3d11.h>
#include <random>
#include <string>

Conways::Conways(uint64_t height, uint64_t width, uint32_t scale, ID3D11Device* pDevice)
  : m_height(height),
    m_width(width),
    m_grid(width, height),
    m_upsampledGrid(width * scale, height * scale),
    m_rule(std::set<uint8_t>{3}, std::set<uint8_t>{2, 3}),
    m_defaultRule(std::set<uint8_t>{3}, std::set<uint8_t>{2, 3}),
    m_neighborhood(NeighborhoodSize::Moore1),
    m_scale(scale),
    m_wrap(true),
    m_pDevice(pDevice),
    m_texture(NULL),
    m_view(NULL)
{
  loadGrid();
}

void Conways::showAutomataWindow()
{
  static int timer = 0;
  static int timerReset = 0;

  static bool displayRuleMenu = false;

  if (ImGui::Button("Show Rule Editor"))
    displayRuleMenu = true;  

  if (displayRuleMenu)
    showRuleMenu(displayRuleMenu);
    
  

  ImGui::Checkbox("Wrap edges", &m_wrap);

  if (ImGui::Button("Start"))
  {
    resetGrid();
  }

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

void Conways::showRuleMenu(bool& show)
{
  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoResize;
  ImGui::Begin("Rule Editor", &show, flags);
  ImGui::Text("Click to toggle rules");
  if (ImGui::Button("Reset rule to default"))
    m_rule = m_defaultRule;
  if (ImGui::Button("Clear rule"))
  {
    m_rule = Rule(std::set<uint8_t>{}, std::set<uint8_t>{});
  }
  ImGui::Text("Birth Conditions:");
  for (uint8_t i = 0; i < 9; i++)
  {
    if (ImGui::Button((std::string("b") + std::to_string(i)).c_str()))
    {
      if (m_rule.m_birthConditions.count(i))
        m_rule.m_birthConditions.erase(i);
      else
        m_rule.m_birthConditions.insert(i);
    }
    if (i != 8)
      ImGui::SameLine();
  }
  ImGui::Text("Survival Conditions:");
  for (uint8_t i = 0; i < 9; i++)
  {
    if (ImGui::Button((std::string("s") + std::to_string(i)).c_str()))
    {
      if (m_rule.m_surviveConditions.count(i))
        m_rule.m_surviveConditions.erase(i);
      else
        m_rule.m_surviveConditions.insert(i);
    }
    if (i != 8)
      ImGui::SameLine();
  }

  ImGui::Text("Survival Conditions:");
  for (uint8_t i = 0; i < 9; i++)
  {
    
    if (i != 8)
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
    if (row <= -1)
      r = m_height + row;
    if (row >= m_height)
      r = m_height - row;
    if (col <= -1)
      c = m_width + col;
    if (col >= m_width)
      c = m_width - col;
  }
  else
  {
    if (row == -1 || row == m_height || col == -1 || col == m_width)
      return false;
  }
  return m_grid.checkCell(r, c);
}