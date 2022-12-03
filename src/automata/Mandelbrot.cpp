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

namespace mandelbrot
{
void mandelbrot::showAutomataWindow(Grid& grid,
                                    FractalBounds window,
                                    Int2 imageSize,
                                    ID3D11ShaderResourceView* view)
{
  static bool displayRuleMenu = false;
  // dumb solution
  static int iterations = 1000;

  static uint32_t mouseX;
  static uint32_t mouseY;

  static bool updateView = true;
  static Smooth smooth = Smooth::Distance;
  static bool debug;
  static float minDistance;

  const char* smoothList[] = {"None", "Linear", "Logarithmic",
                              "Distance Estimate"};
  static int smoothIdx = smooth;
  static int numInterpolatedColors = 2;
  const char* items[] = {"2", "3", "4"};
  static int item_current = 0;

  static ImVec4 color0 = {0, 0, 0, 1};
  static ImVec4 color1 = {1, 1, 1, 1};
  static ImVec4 color2 = {1, 1, 1, 1};
  static ImVec4 color3 = {1, 1, 1, 1};

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

    if (ImGui::ColorEdit4("Inside Color", (float*)&(m_setColor),
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

      if (ImGui::ColorEdit4("Distance Color", (float*)&(m_distanceColor),
                            ImGuiColorEditFlags_NoInputs))
      {
        grid.clear();
        updateView = true;
      }
    }
    else
    {
      if (ImGui::Combo("Number of palette colors", &item_current, items,
                       IM_ARRAYSIZE(items)))
      {
        if (numInterpolatedColors == 2)
          // another dump dum dumb soluton, remember to apply below
          updatePalette({imvec4ToColor(color0), imvec4ToColor(color1)}, 120); 
        if (numInterpolatedColors == 3)
          updatePalette({imvec4ToColor(color0), imvec4ToColor(color1),
                         imvec4ToColor(color2)}, 120);
        if (numInterpolatedColors == 4)
          updatePalette({imvec4ToColor(color0), imvec4ToColor(color1),
                         imvec4ToColor(color2), imvec4ToColor(color3)}, 120);
        grid.clear();
        updateView = true;
      }
      // this needs to be fixed...
      numInterpolatedColors = item_current + 2;
      if (ImGui::ColorEdit4("color 0", (float*)&(color0),
                            ImGuiColorEditFlags_NoInputs) ||
          ImGui::ColorEdit4("color 1", (float*)&(color1),
                            ImGuiColorEditFlags_NoInputs) ||
          ImGui::ColorEdit4("color 2", (float*)&(color2),
                            ImGuiColorEditFlags_NoInputs) ||
          ImGui::ColorEdit4("color 3", (float*)&(color3),
                            ImGuiColorEditFlags_NoInputs))
      {
        if (numInterpolatedColors == 2)
          updatePalette({imvec4ToColor(color0), imvec4ToColor(color1)});
        if (numInterpolatedColors == 3)
          updatePalette({imvec4ToColor(color0), imvec4ToColor(color1),
                         imvec4ToColor(color2)});
        if (numInterpolatedColors == 4)
          updatePalette({imvec4ToColor(color0), imvec4ToColor(color1),
                         imvec4ToColor(color2), imvec4ToColor(color3)});
        grid.clear();
        updateView = true;
      }
    }
    ImGui::End();
  }

  if (updateView)
  {
    updateGrid(numThreads, window, grid, palette, minDistance);
  }
  updateView = false;

  ImGui::Image((void*)view, ImVec2(imageSize.x, imageSize.y));

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
    auto result = calculatePixel(complexX, complexY, smooth, iterations);
    ImGui::Text("Screen space: x:%d, y:%d", mouseX, mouseY);
    ImGui::Text("Complex space: x:%f, y:%f", complexX, complexY);
    ImGui::Text("Iterations: %f", result);
  }
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}

// should be called once per frame
void mandelbrot::loadGrid(Grid& grid,
                          ID3D11Device* device,
                          const Int2 imageSize,
                          ID3D11ShaderResourceView* view,
                          ID3D11Texture2D* texture)
{
  if (texture)
    texture->Release();
  if (view)
    view->Release();

  automata::LoadTextureFromData(grid.getData(), &view, &texture, device,
                                imageSize.x, imageSize.y);
}

void mandelbrot::updateGrid(const Smooth smooth,
                            const uint32_t numThreads,
                            const FractalBounds& window,
                            Grid& rGrid,
                            const Palette& palette,
                            const float minDistance)
{
  std::vector<std::future<bool>> futures;
  for (uint32_t i = 0; i < numThreads; i++)
  {
    futures.push_back(std::async(std::launch::async,
                                 &getMandelbrotPixels,
                                 i,
                                 numThreads,
                                 smooth,
                                 window,
                                 rGrid,
                                 maxIterations,
                                 palette,
                                 minDistance));
  }
  for (auto&& future : futures)
  {
    auto result = future.get();
    if (!result)
      throw std::runtime_error("fractal generation failed");
  }
  loadGrid();
}

Palette mandelbrot::updatePalette(std::vector<Color> colorList, const uint32_t numColors)
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
    p.interpolate(colorList[0], colorList[1], colorList[2],
                          colorList[3]);
  return p;
}

bool mandelbrot::getMandelbrotPixels(const uint32_t offset,
                                     const uint32_t numWorkers,
                                     const Smooth smooth,
                                     const FractalBounds window,
                                     Grid& grid,
                                     const uint32_t maxIterations,
                                     const Int2 imageSize,
                                     Palette palette,
                                     const float minDistance)
{
  for (uint32_t x = 0; x < imageSize.x; x++)
  {
    for (uint32_t y = offset; y < imageSize.y; y += numWorkers)
    {
      if (!grid.checkCell(y, x)) // only update pixels that are empty
      {
        // translate from pixel space to our virtual space
        double v_x =
          window.xmin + ((window.xmax - window.xmin) / imageSize.x) * x;
        double v_y =
          window.ymin + ((window.ymax - window.ymin) / imageSize.y) * y;

        double result = calculatePixel(v_x, v_y, smooth, maxIterations);
        if (result == -1.0)
        {
          grid.setCellDirectly(y, x, imvec4ToColor(m_setColor));
        }
        else
        {
          if (smooth == Smooth::Distance)
          {
            if (result < minDistance)
            {
              grid.setCellDirectly(y, x, imvec4ToColor(m_distanceColor));
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
            grid.setCellDirectly(y, x, newColor);
          }
        }
      }
    }
  }
  return true;
}

double mandelbrot::calculatePixel(const double x_0, const double y_0,
                                  const Smooth smooth,
                                  const uint32_t maxIterations)
{
  // cardioid check
  double p = sqrt((x_0 - 0.25) * (x_0 - 0.25) + y_0 * y_0);
  if (x_0 <= p - (2 * p * p) + 0.25)
    return -1;

  // period 2 bulb check
  if ((x_0 + 1) * (x_0 + 1) + y_0 * y_0 <= 1.0 / 16)
    return -1;

  double z_x = 0;
  double z_y = 0;

  double before_x = 0;
  double before_y = 0;

  double x_2 = 0;
  double y_2 = 0;

  double dz_x = 1;
  double dz_y = 0;

  uint32_t iteration = 0;

  while (x_2 + y_2 < (smooth == Smooth::Logarithmic ? 16 : 4) &&
         iteration < maxIterations)
  {
    before_x = z_x;
    before_y = z_y;

    // iterate: z = z^2 + c
    z_y = 2 * z_x * z_y + y_0;
    z_x = x_2 - y_2 + x_0;
    x_2 = z_x * z_x;
    y_2 = z_y * z_y;

    if (smooth == Smooth::Distance)
    {
      double dz_x_new = 2 * (z_x * dz_x - z_y * dz_y) + 1;
      double dz_y_new = 2 * (z_y * dz_x + z_x * dz_y);

      dz_x = dz_x_new;
      dz_y = dz_y_new;
    }

    iteration++;
  }
  if (iteration == maxIterations)
  {
    return -1; // inside the mandelbrot set
  }
  if (smooth == Smooth::Linear)
  {
    double ratio = (sqrt(x_2 + y_2) - 2) /
                   (2 - sqrt(before_x * before_x + before_y * before_y));
    double gradient = 1 / (ratio + 1);
    return iteration + gradient;
  }
  if (smooth == Smooth::Logarithmic)
  {
    double log_zn = log(sqrt(x_2 + y_2));
    double gradient = 1 - log(log_zn / log(2)) / log(2);
    return iteration + gradient;
  }
  if (smooth == Smooth::Distance)
  {
    double mod_z = x_2 + y_2;
    return mod_z * log(mod_z) / sqrt(dz_x * dz_x + dz_y + dz_y);
  }
  return iteration;
}
}

