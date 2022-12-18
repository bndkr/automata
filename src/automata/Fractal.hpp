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
  uint32_t maxIterations;
  Int2 imageSize;
  ID3D11ShaderResourceView** pView;
  ID3D11Texture2D** pTexture;
  ID3D11Device* pDevice;
  ImVec4 setColor;
  ImVec4 distanceColor;
  FractalType type;
};

namespace fractal
{
  void showAutomataWindow(ID3D11Device* pDevice);

  void loadGrid(Grid* pGrid, ID3D11Device* device, const Int2 imageSize,
                ID3D11ShaderResourceView** pView, ID3D11Texture2D** pTexture);

  void updateGrid(const Smooth smooth, const uint32_t numThreads,
                  const FractalBounds& window, Grid* rGrid, Palette& palette,
                  const float minDistance, const uint32_t maxIterations,
                  const Int2 imageSize, ID3D11ShaderResourceView** pView,
                  ID3D11Texture2D** pTexture, ID3D11Device* pDevice,
                  const ImVec4& setColor, const ImVec4& distanceColor,
                  const FractalType& type);

  Palette updatePalette(std::vector<Color> colorList, const uint32_t numColors);

  bool getFractalPixels(const uint32_t offset, const uint32_t numWorkers,
                           const Smooth smooth, const FractalBounds window,
                           Grid* pGrid, const uint32_t maxIterations,
                           const Int2 imageSize, Palette& palette,
                           const float minDistance, const ImVec4& setColor,
                           const ImVec4& distanceColor, const FractalType& type);
};

#endif
