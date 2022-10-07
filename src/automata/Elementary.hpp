#ifndef AUTOMATA_ELEMENTARY
#define AUTOMATA_ELEMENTARY

#include "Grid.hpp"
#include <d3d11.h>

class Elementary {
public:
  Elementary(uint64_t height, uint64_t width, ID3D11Device* pDevice , ID3D11DeviceContext* pDeviceContext);

  void showAutomataWindow(ID3D11Device *pDevice);

  void updateGrid(bool randInit, bool wrap);

  void upsampleGrid();

  bool checkCell(uint32_t row, uint32_t col, bool wrap);

private:
  uint64_t m_height;
  uint64_t m_width;
  Grid m_grid;
  uint64_t m_upsampledSize;
  std::vector<uint8_t> m_upsampledGrid;
  uint32_t m_scale;
  int m_rule;
  ID3D11ShaderResourceView* m_view;
  ID3D11Texture2D* m_texture;
  ID3D11Device* m_pDevice;
  ID3D11DeviceContext* m_pDeviceContext;
};

#endif
