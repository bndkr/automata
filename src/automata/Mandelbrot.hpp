#ifndef AUTOMATA_MANDELBROT
#define AUTOMATA_MANDELBROT

#include "Grid.hpp"
#include "Palette.hpp"
#include <d3d11.h>

#include <imgui/imgui.h>

enum Smooth
{
  None,
  Linear,
  Logarithmic,
  Distance
};

class Mandelbrot
{
public:
  Mandelbrot(uint64_t height, uint64_t width, ID3D11Device* pDevice);

  void showAutomataWindow();

  void loadGrid();

  void updateGrid(Smooth smooth);

  void updatePalette(std::vector<Color> colorList);

  bool getMandelbrotPixels(uint32_t offset, uint32_t numWorkers, Smooth smooth);

private:

  double calculatePixel(double x, double y, Smooth smooth);
  int64_t m_height;
  int64_t m_width;
  double m_xmin;
  double m_xmax;
  double m_ymin;
  double m_ymax;
  int m_iterations;
  Palette m_palette;
  uint32_t m_numThreads;
  Grid m_grid;
  ID3D11Device* m_pDevice;
  ID3D11ShaderResourceView* m_view;
  ID3D11Texture2D* m_texture;
  bool m_debug;
  float m_minDistance;
  ImVec4 m_setColor;
  ImVec4 m_distanceColor;
};

#endif
