#ifndef AUTOMATA_GRADIENT
#define AUTOMATA_GRADIENT

#include "Grid.hpp"

#include <d3d11.h>  
#include <set>
#include <map>

struct GradientRule
{
  GradientRule(std::pair<int, int> birthConditions,
               std::pair<int, int> surviveConditions)
    : m_birthConditions(birthConditions), m_surviveConditions(surviveConditions)
  {}

  bool survived(uint32_t neighbors)
  {
    return (m_surviveConditions.first <= neighbors &&
            m_surviveConditions.second >= neighbors);
  }

  bool born(uint32_t neighbors)
  {
    return (m_birthConditions.first <= neighbors &&
            m_birthConditions.second >= neighbors);
  }
  std::pair<int, int> m_birthConditions;
  std::pair<int, int> m_surviveConditions;
};

class Gradient {
public:
  Gradient(uint64_t height, uint64_t width, uint32_t scale,
          ID3D11Device* pDevice);

  void showAutomataWindow();

  void showRuleMenu(bool& show);

  void loadGrid();

  void updateGrid();

  void resetGrid();

  uint32_t countNeighbors(int32_t height, int32_t width);

  uint32_t checkCell(int32_t row, int32_t col);

private:
  int64_t m_height;
  int64_t m_width;
  Grid m_grid;
  Grid m_upsampledGrid;
  GradientRule m_rule;
  GradientRule m_defaultRule;
  uint32_t m_scale;
  uint32_t m_neighborhoodSize;
  std::map<std::string, GradientRule> m_presetRules;
  bool m_wrap;
  ID3D11Device* m_pDevice;
  ID3D11ShaderResourceView* m_view;
  ID3D11Texture2D* m_texture;
};

#endif