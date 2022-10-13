#ifndef AUTOMATA_CONWAYS
#define AUTOMATA_CONWAYS

#include "Grid.hpp"
#include <d3d11.h>  

class Conways {
public:
  Conways(uint64_t height, uint64_t width, uint32_t scale,
          ID3D11Device* pDevice);

  void showAutomataWindow();

  void displayGrid();

  void updateGrid();

  bool checkCell(uint32_t row, uint32_t col, bool wrap);

private:
  uint64_t m_height;
  uint64_t m_width;
  Grid m_grid;
  Grid m_upsampledGrid;
  uint32_t m_scale;
  ID3D11Device* m_pDevice;
  ID3D11ShaderResourceView* m_view;
  ID3D11Texture2D* m_texture;
};


#endif