#ifndef AUTOMATA_CONWAYS
#define AUTOMATA_CONWAYS

#include "Grid.hpp"

#include <d3d11.h>  
#include <set>
#include <map>

struct Rule
{
  Rule(std::set<uint8_t>& birthConditions,
       std::set<uint8_t>& surviveConditions)
    : m_birthConditions(birthConditions), m_surviveConditions(surviveConditions)
  {}

  bool survived(uint8_t neighbors)
  {
    return m_surviveConditions.count(neighbors);
  }

  bool born(uint8_t neighbors)
  {
    return m_birthConditions.count(neighbors);
  }
  std::set<uint8_t> m_birthConditions;
  std::set<uint8_t> m_surviveConditions;
};

class Conways {
public:
  Conways(uint64_t height, uint64_t width, uint32_t scale,
          ID3D11Device* pDevice);

  void showAutomataWindow();

  void showRuleMenu(bool& show);

  void loadGrid();

  void updateGrid();

  void resetGrid();

  uint32_t countNeighbors(int32_t height, int32_t width);

  bool checkCell(int32_t row, int32_t col);

private:
  int64_t m_height;
  int64_t m_width;
  Grid m_grid;
  Grid m_upsampledGrid;
  Rule m_rule;
  Rule m_defaultRule;
  uint32_t m_scale;
  uint32_t m_neighborhoodSize;
  std::map<std::string, Rule> m_presetRules;
  bool m_wrap;
  ID3D11Device* m_pDevice;
  ID3D11ShaderResourceView* m_view;
  ID3D11Texture2D* m_texture;
};

#endif