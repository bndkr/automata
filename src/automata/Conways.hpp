#ifndef AUTOMATA_CONWAYS
#define AUTOMATA_CONWAYS

#include "Grid.hpp"
#include <d3d11.h>  

class Conways {
public:
  Conways(uint64_t height, uint64_t width);

  void showAutomataWindow(ID3D11Device* pDevice);

  void displayGrid(bool randInit, bool wrap, ID3D11Device* pDevice, ID3D11ShaderResourceView* view);

  void updateGrid(bool randInit, bool wrap);

  void upsampleGrid();

  bool checkCell(uint32_t row, uint32_t col, bool wrap);

private:
  uint64_t m_height;
  uint64_t m_width;
  Grid m_grid;
  std::vector<uint8_t> m_upsampledGrid;
  uint32_t m_scale;
  int m_rule;
};


#endif