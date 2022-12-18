#include "Fractal.hpp"
#include "Mandelbrot.hpp"

#include "imgui/imgui.h"
#include "utils/LoadTextureFromData.hpp"

#include <d3d11.h>
#include <string>
#include <future>

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
Palette updatePalette(std::vector<Color> colorList,
                                  const uint32_t numColors)
{
  auto p = Palette(numColors);
  if (colorList.size() < 2 || colorList.size() > 4)
  {
    throw std::runtime_error("cannot interpolate between" +
                             std::to_string(colorList.size()) + " colors");
  }

  if (colorList.size() == 2)
    p.interpolate(colorList[0], colorList[1]);
  if (colorList.size() == 3)
    p.interpolate(colorList[0], colorList[1], colorList[2]);
  if (colorList.size() == 4)
    p.interpolate(colorList[0], colorList[1], colorList[2], colorList[3]);
  return p;
}
void showAutomataWindow(ID3D11Device* pDevice)
{
  static Grid grid(1000, 500);
  static FractalBounds window{-2.0, 2.0, -1.0, 1.0};
  static Int2 imageSize{1000, 500};

  static bool displayRuleMenu = false;
  static int iterations = 1000;

  static uint32_t mouseX;
  static uint32_t mouseY;

  static bool updateView = true;
  static Smooth smooth = Smooth::Linear;
  static bool debug = true;
  static float minDistance = 0.01f;

  static ID3D11ShaderResourceView* pView = NULL;
  static ID3D11Texture2D* pTexture = NULL;

  static FractalType type = FractalType::Mandelbrot;
  
  static ImVec4 setColor = {0, 0, 0, 1};
  static ImVec4 distanceColor = {1, 1, 1, 1};

  static uint32_t numColors = 120;

  static Palette palette(numColors);
  static uint32_t numThreads(std::thread::hardware_concurrency());

  const char* smoothList[] = {"None", "Linear", "Logarithmic",
                              "Distance Estimate"};
  static int smoothIdx = (int) smooth;
  static int numInterpolatedColors = 2;
  const char* items[] = {"2", "3", "4"};
  static int item_current = 0;

  static bool showPalette = true;
  ImGui::Checkbox("Show Palette", &showPalette);

  if (showPalette)
  {
    for (uint32_t i = 0; i < numColors; i++)
    {
      ImGui::Text("%d: r: %d, g: %d, b:%d", i, palette.getColor(i).r,
                  palette.getColor(i).g, palette.getColor(i).b);
    }
  }


  static std::vector<ImVec4> colors = {
    {1.0, 1.0, 1.0, 1.0},
    {1.0, 1.0, 1.0, 1.0},
    {1.0, 1.0, 1.0, 1.0},
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

    if (ImGui::SliderInt("Iterations", &iterations, 0, 2000))
    {
      grid.clear();
      updateView = true;
    }

    if (ImGui::Combo("Smoothing Algorithm", &smoothIdx, smoothList,
                     IM_ARRAYSIZE(smoothList)))
    {
      smooth = (Smooth)smoothIdx;
      updateView = true;
      grid.clear();
    }

    if (ImGui::ColorEdit4("Inside Color", (float*)&(setColor),
                          ImGuiColorEditFlags_NoInputs))
    {
      updateView = true;
      grid.clear();
    }

    if (smooth == Smooth::Distance)
    {
      if (ImGui::SliderFloat("Distance", &minDistance, 0.0f, 0.1f, "%.12f",
                             ImGuiSliderFlags_Logarithmic))
      {
        grid.clear();
        updateView = true;
      }

      if (ImGui::ColorEdit4("Distance Color", (float*)&(distanceColor),
                            ImGuiColorEditFlags_NoInputs))
      {
        grid.clear();
        updateView = true;
      }
    }
    else
    {
      const char* items[] = {"2", "3", "4"};
      static int item_current = 0;
      if (ImGui::Combo("Number of palette colors", &item_current, items,
                       IM_ARRAYSIZE(items)))
      {
        if (numInterpolatedColors == 2)
          updatePalette({imvec4ToColor(colors[0]), imvec4ToColor(colors[1])},
                        numColors);
        if (numInterpolatedColors == 3)
          updatePalette({imvec4ToColor(colors[0]), imvec4ToColor(colors[1]),
                         imvec4ToColor(colors[2])},
                        numColors);
        if (numInterpolatedColors == 4)
          updatePalette({imvec4ToColor(colors[0]), imvec4ToColor(colors[1]),
                         imvec4ToColor(colors[2]), imvec4ToColor(colors[3])},
                        numColors);
        updateView = true;
        grid.clear();
      }

      numInterpolatedColors = item_current + 2;
      for (int i = 0; i < numInterpolatedColors; i++)
      {
        if (ImGui::ColorEdit4(
              std::string("color " + std::to_string(i + 1)).c_str(),
              (float*)&(colors[i]), ImGuiColorEditFlags_NoInputs))
        {
          if (numInterpolatedColors == 2)
             palette = updatePalette({imvec4ToColor(colors[0]), imvec4ToColor(colors[1])},
                          numColors);
          if (numInterpolatedColors == 3)
            palette = updatePalette({imvec4ToColor(colors[0]), imvec4ToColor(colors[1]),
                           imvec4ToColor(colors[2])}, numColors);
          if (numInterpolatedColors == 4)
            palette = updatePalette({imvec4ToColor(colors[0]), imvec4ToColor(colors[1]),
                           imvec4ToColor(colors[2]), imvec4ToColor(colors[3])},
                          numColors);
          updateView = true;
          grid.clear();
        }
      }
    }
    ImGui::End();
  }

  if (updateView)
  {
    updateGrid(smooth, numThreads, window, &grid, palette, minDistance,
               iterations, imageSize, &pView, &pTexture, pDevice, setColor, distanceColor, type);
  }
  // updateView = false;

  ImGui::Image((void*)pView, ImVec2(imageSize.x, imageSize.y));

  auto mousePositionAbsolute = ImGui::GetMousePos();
  auto screenPositionAbsolute = ImGui::GetItemRectMin();

  bool hovering = ImGui::IsItemHovered();
  bool mouseDragging = ImGui::IsMouseDragging(ImGuiMouseButton_Left);

  mouseX = mousePositionAbsolute.x - screenPositionAbsolute.x;
  mouseY = mousePositionAbsolute.y - screenPositionAbsolute.y;

  double complexX =
    (mouseX * ((window.xmax - window.xmin) / imageSize.x)) + window.xmin;
  double complexY =
    -((mouseY * ((window.ymax - window.ymin) / imageSize.y)) + window.ymin);
  // negative because the top left corner is (0,0), not the bottom left corner

  static bool wasDragging = false;

  if (hovering && mouseDragging)
  {
    auto dragDelta = ImGui::GetMouseDragDelta();
    ImGui::ResetMouseDragDelta();
    if (dragDelta.x != 0 || dragDelta.y != 0)
    {
      auto complexDiffX =
        dragDelta.x * ((window.xmax - window.xmin) / imageSize.x);
      auto complexDiffY =
        dragDelta.y * ((window.ymax - window.ymin) / imageSize.y);

      window.xmin -= complexDiffX;
      window.xmax -= complexDiffX;
      window.ymin -= complexDiffY;
      window.ymax -= complexDiffY;

      updateView = true;
      wasDragging = true;
      grid.translate(dragDelta.x, dragDelta.y);
    }
  }
  // zoom in
  if (hovering && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !wasDragging)
  {
    window.xmin = (window.xmin + complexX) / 2;
    window.xmax = (window.xmax + complexX) / 2;

    window.ymin = (window.ymin - complexY) / 2;
    window.ymax = (window.ymax - complexY) / 2;
    grid.clear();
    updateView = true;
  }
  // zoom out
  if (hovering && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
  {
    window.xmin -= (complexX - window.xmin);
    window.xmax += (window.xmax - complexX);

    window.ymin += (complexY + window.ymin);
    window.ymax += (window.ymax + complexY);
    grid.clear();
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
void loadGrid(Grid* pGrid,
                          ID3D11Device* device,
                          const Int2 imageSize,
                          ID3D11ShaderResourceView** pView,
                          ID3D11Texture2D** pTexture)
{
  if (*pTexture)
    (*pTexture)->Release();
  if (*pView)
    (*pView)->Release();

  automata::LoadTextureFromData(pGrid->getData(), pView, pTexture, device,
                                imageSize.x, imageSize.y);
}

void updateGrid(const Smooth smooth, const uint32_t numThreads,
                const FractalBounds& window, Grid* pGrid, Palette& palette,
                const float minDistance, const uint32_t maxIterations,
                const Int2 imageSize, ID3D11ShaderResourceView** pView,
                ID3D11Texture2D** pTexture, ID3D11Device* pDevice,
                const ImVec4& setColor, const ImVec4& distanceColor,
                const FractalType& type)
{
  std::vector<std::future<bool>> futures;
  for (uint32_t i = 0; i < numThreads; i++)
  {
    futures.push_back(std::async(std::launch::async, &getFractalPixels, i,
                                 numThreads, smooth, window, pGrid,
                                 maxIterations, imageSize, palette, minDistance,
                                 setColor, distanceColor, type));
  }
  for (auto&& future : futures)
  {
    auto result = future.get();
    if (!result)
      throw std::runtime_error("fractal generation failed");
  }
  loadGrid(pGrid, pDevice, imageSize, pView, pTexture);
}

bool getFractalPixels(
  const uint32_t offset, const uint32_t numWorkers, const Smooth smooth,
  const FractalBounds window, Grid* pGrid, const uint32_t maxIterations,
  const Int2 imageSize, Palette& palette, const float minDistance,
  const ImVec4& setColor, const ImVec4& distanceColor, const FractalType& type)
{
  for (uint32_t x = 0; x < imageSize.x; x++)
  {
    for (uint32_t y = offset; y < imageSize.y; y += numWorkers)
    {
      if (!pGrid->checkCell(y, x)) // only update pixels that are empty
      {
        // translate from pixel space to our virtual space
        double v_x =
          window.xmin + ((window.xmax - window.xmin) / imageSize.x) * x;
        double v_y =
          window.ymin + ((window.ymax - window.ymin) / imageSize.y) * y;

        double result = 0;
        if (type == FractalType::Mandelbrot)
          result = mandelbrot::calculatePixel(v_x, v_y, smooth, maxIterations);
        else if (type == FractalType::Julia)
          result = 0;
        if (result == -1.0)
        {
          pGrid->setCellDirectly(y, x, imvec4ToColor(setColor));
        }
        else
        {
          if (smooth == Smooth::Distance)
          {
            if (result < minDistance)
            {
              pGrid->setCellDirectly(y, x, imvec4ToColor(distanceColor));
            }
          }
          else
          {
            result = normalizeIteration(result);
            double gradient = result - (int)result;
            int step = (int)result % palette.numColors;
            int nextStep = (step + 1) % palette.numColors;
            auto baseColor = palette.getColor(step);
            auto nextColor = palette.getColor(nextStep);
            // interpolate between baseColor and nextColor
            auto newColor = Color{
              (uint8_t)(baseColor.r + (nextColor.r - baseColor.r) * gradient),
              (uint8_t)(baseColor.g + (nextColor.g - baseColor.g) * gradient),
              (uint8_t)(baseColor.b + (nextColor.b - baseColor.b) * gradient),
              255};
            pGrid->setCellDirectly(y, x, newColor);
          }
        }
      }
    }
  }
  return true;
}


}

