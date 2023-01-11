#include "Fractal.hpp"
#include "Mandelbrot.hpp"

#include "imgui/imgui.h"
#include "utils/LoadTextureFromData.hpp"

#include <d3d11.h>
#include <future>
#include <string>

// non-optimized
#include <complex>

namespace
{
Color imvec4ToColor(ImVec4 vec)
{
  uint8_t red = vec.x * 255;
  uint8_t green = vec.y * 255;
  uint8_t blue = vec.z * 255;
  uint8_t alpha = vec.w * 255;
  return Color{red, green, blue, alpha};
}

double normalizeIteration(double input)
{
  return (input * log(input + 1)) / sqrt(input);
}
} // namespace

namespace fractal
{
Palette updatePalette(std::vector<Color> colorList, const uint32_t numColors)
{
  auto p = Palette(colorList, numColors);
  return p;
}
void showAutomataWindow(ID3D11Device* pDevice)
{
  std::vector<Color> initialColors = {
    Color{0, 0, 0, 255}, Color{255, 255, 255, 255}};
  static Grid grid(1000, 500);
  static Smooth smooth = Smooth::Logarithmic;
  static uint32_t numThreads(std::thread::hardware_concurrency());
  static uint32_t numColors = 120;
  static int numSteps = 100;
  static int numPaletteColors = 2;
  static Palette palette(initialColors, numSteps);
  static float minDistance = 0.01f;
  static int iterations = 1000;
  static Int2 imageSize{1000, 500};
  static ID3D11ShaderResourceView* pView = NULL;
  static ID3D11Texture2D* pTexture = NULL;
  static ImVec4 setColor = {0, 0, 0, 1};
  static ImVec4 distanceColor = {1, 1, 1, 1};
  static FractalType type = FractalType::Mandelbrot;
  static FractalBounds window{-2.0, 2.0, -1.0, 1.0};

  static FractalInfo f{&grid,       smooth,     numThreads, &palette,
                       minDistance, iterations, imageSize,  &pView,
                       &pTexture,   pDevice,    setColor,   distanceColor,
                       type,        window};

  static bool displayRuleMenu = false;

  static uint32_t mouseX;
  static uint32_t mouseY;

  static bool updateView = true;
  static bool debug = true;

  const char* smoothList[] = {"None", "Linear", "Logarithmic",
                              "Distance Estimate"};
  static int smoothIdx = (int)smooth;
  static int numInterpolatedColors = 2;
  const char* items[] = {"2", "3", "4"};
  static int item_current = 0;

  static bool showPalette = true;
  ImGui::Checkbox("Show Palette", &showPalette);

  static std::vector<ImVec4> colors = {
    {0.0, 0.0, 0.0, 1.0},
    {1.0, 1.0, 1.0, 1.0},
    {0.0, 0.0, 0.0, 1.0},
    {1.0, 1.0, 1.0, 1.0},
  };

  if (ImGui::Button("Fractal Options"))
    displayRuleMenu = true;
  if (displayRuleMenu)
  {
    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_AlwaysAutoResize;
    flags |= ImGuiWindowFlags_NoResize;
    ImGui::Begin("Fractal Options", &displayRuleMenu, flags);

    ImGui::Checkbox("Debug Info", &debug);

    if (ImGui::SliderInt("Iterations", &f.maxIterations, 0, 2000))
    {
      f.pGrid->clear();
      updateView = true;
    }

    if (ImGui::Combo("Smoothing Algorithm", &smoothIdx, smoothList,
                     IM_ARRAYSIZE(smoothList)))
    {
      f.smooth = (Smooth)smoothIdx;
      updateView = true;
      f.pGrid->clear();
    }

    if (f.smooth == Smooth::Distance)
    {
      if (ImGui::SliderFloat("Distance", &f.minDistance, 0.0000000001f, 0.1f,
                             "%.12f", ImGuiSliderFlags_Logarithmic))
      {
        updateView = true;
        f.pGrid->clear();
      }
      if (ImGui::ColorEdit4("Distance Color", (float*)&f.distanceColor),
          ImGuiColorEditFlags_NoInputs)
      {
        updateView = true;
        f.pGrid->clear();
      }
    }

    if (ImGui::ColorEdit4("Inside Color", (float*)&(f.setColor),
                          ImGuiColorEditFlags_NoInputs))
    {
      updateView = true;
      f.pGrid->clear();
    }

    if (smooth == Smooth::Distance)
    {
      if (ImGui::SliderFloat("Distance", &f.minDistance, 0.0f, 0.1f, "%.12f",
                             ImGuiSliderFlags_Logarithmic))
      {
        f.pGrid->clear();
        updateView = true;
      }

      if (ImGui::ColorEdit4("Distance Color", (float*)&f.distanceColor,
                            ImGuiColorEditFlags_NoInputs))
      {
        f.pGrid->clear();
        updateView = true;
      }
    }
    else
    {
      if (ImGui::InputInt("Number of colors", &numPaletteColors, 1))
      {
        // we need to make sure the number of colors
        // divides the number of steps
        numSteps -= (numSteps % numPaletteColors);
        if (numPaletteColors > colors.size())
        {
          for (int i = 0; i < numPaletteColors - colors.size(); i++)
          {
            colors.push_back(ImVec4{1.0f, 1.0f, 1.0f, 1.0f});
          }
        }
        if (numPaletteColors < colors.size())
        {
          for (int i = 0; i < colors.size() - numPaletteColors; i++)
          {
            colors.pop_back();
          }
        }
        f.palette->updateColors(colors);
        updateView = true;
        f.pGrid->clear();
      }

      for (int i = 0; i < numPaletteColors; i++)
      {
        if (ImGui::ColorEdit4(
              std::string("color " + std::to_string(i + 1)).c_str(),
              (float*)&(colors[i]), ImGuiColorEditFlags_NoInputs))
        {
          f.palette->updateColors(colors);
          updateView = true;
          f.pGrid->clear();
        }
      }
    }
    if (ImGui::SliderInt("Palette Steps", &numSteps, 10, 2000, "%d",
                         ImGuiSliderFlags_Logarithmic))
    {
      // we need to make sure the number of colors
      // divides the number of steps
      numSteps -= (numSteps % numPaletteColors);
      f.palette->updateSize(numSteps);
      updateView = true;
      f.pGrid->clear();
    }
    ImGui::End();
  }

  if (updateView)
  {
    updateGrid(f);
  }
  updateView = false;

  ImGui::Image((void*)(*f.pView), ImVec2(f.imageSize.x, f.imageSize.y));

  auto mousePositionAbsolute = ImGui::GetMousePos();
  auto screenPositionAbsolute = ImGui::GetItemRectMin();

  bool hovering = ImGui::IsItemHovered();
  bool mouseDragging = ImGui::IsMouseDragging(ImGuiMouseButton_Left);

  mouseX = mousePositionAbsolute.x - screenPositionAbsolute.x;
  mouseY = mousePositionAbsolute.y - screenPositionAbsolute.y;

  double complexX =
    (mouseX * ((f.window.xmax - f.window.xmin) / f.imageSize.x)) +
    f.window.xmin;
  double complexY =
    -((mouseY * ((f.window.ymax - f.window.ymin) / f.imageSize.y)) +
      f.window.ymin);
  // negative because the top left corner is (0,0), not the bottom left corner

  static bool wasDragging = false;

  if (hovering && mouseDragging)
  {
    auto dragDelta = ImGui::GetMouseDragDelta();
    ImGui::ResetMouseDragDelta();
    if (dragDelta.x != 0 || dragDelta.y != 0)
    {
      auto complexDiffX =
        dragDelta.x * ((f.window.xmax - f.window.xmin) / f.imageSize.x);
      auto complexDiffY =
        dragDelta.y * ((f.window.ymax - f.window.ymin) / f.imageSize.y);

      f.window.xmin -= complexDiffX;
      f.window.xmax -= complexDiffX;
      f.window.ymin -= complexDiffY;
      f.window.ymax -= complexDiffY;

      updateView = true;
      wasDragging = true;
      grid.translate(dragDelta.x, dragDelta.y);
    }
  }
  // zoom in
  if (hovering && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !wasDragging)
  {
    f.window.xmin = (f.window.xmin + complexX) / 2;
    f.window.xmax = (f.window.xmax + complexX) / 2;

    f.window.ymin = (f.window.ymin - complexY) / 2;
    f.window.ymax = (f.window.ymax - complexY) / 2;
    f.pGrid->clear();
    updateView = true;
  }
  // zoom out
  if (hovering && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
  {
    f.window.xmin -= (complexX - f.window.xmin);
    f.window.xmax += (f.window.xmax - complexX);

    f.window.ymin += (complexY + f.window.ymin);
    f.window.ymax += (f.window.ymax + complexY);
    f.pGrid->clear();
    updateView = true;
  }

  if (ImGui::IsMouseReleased(ImGuiMouseButton_Left && wasDragging))
  {
    wasDragging = false;
  }

  if (hovering && debug)
  {
    ImGui::Text("Screen space: x:%d, y:%d", mouseX, mouseY);
    ImGui::Text("Complex space: x:%f, y:%f", complexX, complexY);
  }
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}

// should be called once per frame
void loadGrid(FractalInfo& f)
{
  if (*f.pTexture)
    (*f.pTexture)->Release();
  if (*f.pView)
    (*f.pView)->Release();

  automata::LoadTextureFromData(f.pGrid->getData(), f.pView, f.pTexture,
                                f.pDevice, f.imageSize.x, f.imageSize.y);
}

void updateGrid(FractalInfo& f)
{
  std::vector<std::future<bool>> futures;
  for (uint32_t i = 0; i < f.numThreads; i++)
  {
    futures.push_back(std::async(std::launch::async, &getFractalPixels, i, f));
  }
  for (auto&& future : futures)
  {
    auto result = future.get();
    if (!result)
      throw std::runtime_error("fractal generation failed");
  }
  loadGrid(f);
}

bool getFractalPixels(uint32_t offset, FractalInfo& f)
{
  for (uint32_t x = 0; x < f.imageSize.x; x++)
  {
    for (uint32_t y = offset; y < f.imageSize.y; y += f.numThreads)
    {
      if (!f.pGrid->checkCell(y, x)) // only update pixels that are empty
      {
        // translate from pixel space to our virtual space
        double v_x =
          f.window.xmin + ((f.window.xmax - f.window.xmin) / f.imageSize.x) * x;
        double v_y =
          f.window.ymin + ((f.window.ymax - f.window.ymin) / f.imageSize.y) * y;

        double result = 0;
        if (f.type == FractalType::Mandelbrot)
          result =
            mandelbrot::calculatePixel(v_x, v_y, f.smooth, f.maxIterations);
        else if (f.type == FractalType::Julia)
          result = 0;
        if (result == -1.0)
        {
          f.pGrid->setCellDirectly(y, x, imvec4ToColor(f.setColor));
        }
        else
        {
          if (f.smooth == Smooth::Distance)
          {
            if (result < f.minDistance)
            {
              f.pGrid->setCellDirectly(y, x, imvec4ToColor(f.distanceColor));
            }
          }
          else
          {
            result = normalizeIteration(result);
            f.pGrid->setCellDirectly(y, x, f.palette->getColor(result));
          }
        }
      }
    }
  }
  return true;
}
} // namespace fractal
