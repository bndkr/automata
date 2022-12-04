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

struct FractalBounds
{
  double xmin;
  double xmax;
  double ymin;
  double ymax;
};

struct Int2
{
  uint32_t x;
  uint32_t y;
};

namespace mandelbrot
{
  // mandelbrot(uint64_t height, uint64_t width, ID3D11Device* pDevice);

  void showAutomataWindow(Grid& grid, FractalBounds window, Int2 imageSize,
                        ID3D11ShaderResourceView* view);

  void loadGrid(Grid& grid, ID3D11Device* device, const Int2 imageSize,
                ID3D11ShaderResourceView* view, ID3D11Texture2D* texture);

  void updateGrid(const Smooth smooth, const uint32_t numThreads,
                  const FractalBounds& window, Grid& rGrid, Palette& palette,
                  const float minDistance, const uint32_t maxIterations,
                  const Int2 imageSize, ID3D11ShaderResourceView* pView,
                  ID3D11Texture2D* pTexture, ID3D11Device* pDevice);

  Palette updatePalette(std::vector<Color> colorList, const uint32_t numColors);

  bool getMandelbrotPixels(const uint32_t offset, const uint32_t numWorkers,
                           const Smooth smooth, const FractalBounds window,
                           Grid& grid, const uint32_t maxIterations,
                           const Int2 imageSize, Palette& palette,
                           const float minDistance);

  double calculatePixel(const double x_0, const double y_0, const Smooth smooth,
                        const uint32_t maxIterations);

  // int64_t m_height;
  // int64_t m_width;
  // double m_xmin;
  // double m_xmax;
  // double m_ymin;
  // double m_ymax;
  // int m_iterations;
  // Palette m_palette;
  // uint32_t m_numThreads;
  // Grid m_grid;
  // ID3D11Device* m_pDevice;
  // ID3D11ShaderResourceView* m_view;
  // ID3D11Texture2D* m_texture;
  float m_minDistance;
  ImVec4 m_setColor;
  ImVec4 m_distanceColor;
};

#endif
