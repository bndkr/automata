#include "Elementary.hpp"

#include "imgui/imgui.h"
#include "utils/LoadTextureFromData.hpp"

#include <d3d11.h>
#include <random>

Elementary::Elementary(uint64_t height, uint64_t width, ID3D11Device *pDevice)
  : m_height(height),
    m_width(width),
    m_grid(width, height),
    m_upsampledGrid(),
    m_scale(5),
    m_rule(30),
    m_pDevice(pDevice),
    m_view(NULL),
    m_texture(NULL)
{
  m_upsampledSize = 4 * m_width * m_scale * m_height * m_scale;
  m_upsampledGrid.reserve(m_upsampledSize);
  for (uint64_t i = 0; i < m_upsampledSize; i++)
  {
    m_upsampledGrid.push_back(0);
  }

  updateGrid(true, true);
  upsampleGrid();

  automata::LoadTextureFromData(m_upsampledGrid.data(), &m_view, &m_texture,
                                pDevice, m_width * m_scale, m_height * m_scale);
}

void Elementary::showAutomataWindow()
{
  static bool randomInit = true;
  static bool wrap = true;

  if (ImGui::Checkbox("Random Start", &randomInit))
  {
    updateTexture(wrap, randomInit);
  }

  if (ImGui::Checkbox("Wrap", &wrap))
  {
    updateTexture(wrap, randomInit);
  }

  if (ImGui::InputInt("Rule", &m_rule))
  {
    updateTexture(wrap, randomInit);
  }

  ImGui::Image((void *)m_view, ImVec2(m_grid.getWidth() * m_scale,
                                      m_grid.getHeight() * m_scale));
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}

void Elementary::updateGrid(bool randInit, bool wrap)
{
  Color white{255, 255, 255, 255};
  m_grid.clear();
  if (randInit)
  {
    for (uint32_t i = 0; i < m_width; i++)
    {
      if (rand() % 2 == 1)
      {
        m_grid.setCell(0, i, white);
      }
    }
  }
  else
  {
    m_grid.setCell(0, m_width / 2, white);
  }
  m_grid.applyChanges();

  for (uint32_t row = 1; row < m_height; row++)
  {
    for (uint32_t col = 0; col < m_width; col++)
    {
      bool left = checkCell(row - 1, col - 1, wrap);
      bool middle = checkCell(row - 1, col, wrap);
      bool right = checkCell(row - 1, col + 1, wrap);

      if (!left && !middle && !right && (m_rule & 1) ||
          !left && !middle && right && (m_rule & 2) ||
          !left && middle && !right && (m_rule & 4) ||
          !left && middle && right && (m_rule & 8) ||
          left && !middle && !right && (m_rule & 16) ||
          left && !middle && right && (m_rule & 32) ||
          left && middle && !right && (m_rule & 64) ||
          left && middle && right && (m_rule & 128))
      {
        m_grid.setCell(row, col, white);
      }
    }
    m_grid.applyChanges();
  }
}

void Elementary::updateTexture(bool wrap, bool rand)
{
  updateGrid(rand, wrap);
  upsampleGrid();

  m_view->Release();
  m_texture->Release();

  automata::LoadTextureFromData(m_upsampledGrid.data(), &m_view, &m_texture,
                                m_pDevice, m_width * m_scale,
                                m_height * m_scale);
}

// TODO: optimize this
void Elementary::upsampleGrid()
{
  uint64_t newWidth = m_width * m_scale;
  uint64_t newHeight = m_height * m_scale;

  for (uint64_t i = 0; i < newHeight; i++)
  {
    for (uint64_t j = 0; j < newWidth; j++)
    {
      m_upsampledGrid[(j + (i * newWidth)) * 4] =
        m_grid.getArray()[((j / m_scale) + (i / m_scale) * m_width) * 4];
      m_upsampledGrid[(j + (i * newWidth)) * 4 + 1] =
        m_grid.getArray()[((j / m_scale) + (i / m_scale) * m_width) * 4 + 1];
      m_upsampledGrid[(j + (i * newWidth)) * 4 + 2] =
        m_grid.getArray()[((j / m_scale) + (i / m_scale) * m_width) * 4 + 2];
      m_upsampledGrid[(j + (i * newWidth)) * 4 + 3] =
        m_grid.getArray()[((j / m_scale) + (i / m_scale) * m_width) * 4 + 3];
    }
  }
}

bool Elementary::checkCell(uint32_t row, uint32_t col, bool wrap)
{
  if (wrap)
  {
    if (col == -1)
      return m_grid.checkCell(row, m_width - 1);
    if (col == m_width)
      return m_grid.checkCell(row, 0);

    return m_grid.checkCell(row, col);
  }
  else
  {
    if (row < 0 || row >= m_height)
      return false;
    if (col < 0 || col >= m_width)
      return false;
    return m_grid.checkCell(row, col);
  }
}