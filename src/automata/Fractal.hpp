#ifndef AUTOMATA_FRACTAL
#define AUTOMATA_FRACTAL

#include "Grid.hpp"
#include "Palette.hpp"
#include <d3d11.h>

#include <imgui/imgui.h>

enum class Smooth
{
  None,
  Linear,
  Logarithmic,
  Distance
};

struct FractalBounds
{
  double xmin;
  double xmax;
  double ymin;
  double ymax;
};

enum class FractalType
{
  Mandelbrot,
  Julia
};

struct Int2
{
  uint32_t x;
  uint32_t y;
};

struct FractalInfo
{
  Grid* pGrid;
  Smooth smooth;
  uint32_t numThreads;
  Palette* palette;
  float minDistance;
  int maxIterations;
  Int2 imageSize;
  ID3D11ShaderResourceView** pView;
  ID3D11Texture2D** pTexture;
  ID3D11Device* pDevice;
  ImVec4 setColor;
  ImVec4 distanceColor;
  FractalType type;
  FractalBounds window;
  float seedX;
  float seedY;
};

namespace fractal
{
  void showAutomataWindow(ID3D11Device* pDevice);

  void loadGrid(FractalInfo& f);

  void updateGrid(FractalInfo& f);

  Palette updatePalette(std::vector<Color> colorList, const uint32_t numColors);

  bool getFractalPixels(uint32_t offset, FractalInfo& f);
};

#endif
