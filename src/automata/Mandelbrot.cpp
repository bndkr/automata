#include "Mandelbrot.hpp"

#include "imgui/imgui.h"
#include "utils/LoadTextureFromData.hpp"

#include <d3d11.h>
#include <string>
#include <future>

namespace
{
Color imvec4ToColor(ImVec4 vec)
{
  uint8_t red = vec.z * 255;
  uint8_t green = vec.y * 255;
  uint8_t blue = vec.x * 255;
  uint8_t alpha = vec.w * 255;
  return Color{red, green, blue, alpha};
}

double normalizeIteration(double input)
{
  return (input * log(input + 1)) / sqrt(input);
}
} // namespace

Mandelbrot::Mandelbrot(uint64_t height, uint64_t width, ID3D11Device* pDevice)
  : m_height(height),
    m_width(width),
    m_xmin(-2.0),
    m_xmax(2.0),
    m_ymin(-1.0),
    m_ymax(1.0),
    m_iterations(50),
    m_palette(120), // this must be divisible by 12 (3 and 4)
    m_numThreads(std::thread::hardware_concurrency()),
    m_grid(width, height),
    m_pDevice(pDevice),
    m_texture(NULL),
    m_view(NULL)
{
  loadGrid();
}

void Mandelbrot::showAutomataWindow()
{
  static bool displayRuleMenu = false;

  static uint32_t mouseX;
  static uint32_t mouseY;

  static bool updateView = true;
  static Smooth smooth = Smooth::Logarithmic;
  static bool debug;

  const char* smoothList[] = {"None", "Linear", "Logarithmic", "Distance"};
  static int smoothIdx = smooth;
  static int numInterpolatedColors = 2;
  const char* items[] = {"2", "3", "4"};
  static int item_current = 0;

  static std::vector<ImVec4> colors = {
    {0.0, 0.0, 0.0, 1.0},
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
    
    if (ImGui::Combo("Smoothing Algorithm", &smoothIdx, smoothList,
                     IM_ARRAYSIZE(smoothList)))
    {
      smooth = (Smooth)smoothIdx;
      updateView = true;
      m_grid.clear();
    }

    ImGui::Checkbox("Debug Info", &debug);


    if (ImGui::SliderInt("Iterations", &m_iterations, 0, 2000))
    {
      m_grid.clear();
      updateView = true;
    }
    ImGui::Text("Palette options");
    
    if (ImGui::Combo("Number of palette colors", &item_current, items,
                     IM_ARRAYSIZE(items)))
    {
      if (numInterpolatedColors == 2)
        updatePalette({imvec4ToColor(colors[0]), imvec4ToColor(colors[1])});
      if (numInterpolatedColors == 3)
        updatePalette({imvec4ToColor(colors[0]), imvec4ToColor(colors[1]),
                       imvec4ToColor(colors[2])});
      if (numInterpolatedColors == 4)
        updatePalette({imvec4ToColor(colors[0]), imvec4ToColor(colors[1]),
                       imvec4ToColor(colors[2]), imvec4ToColor(colors[3])});
      m_grid.clear();
      updateView = true;
    }
    numInterpolatedColors = item_current + 2;
    for (int i = 0; i < numInterpolatedColors; i++)
    {
      if (ImGui::ColorEdit4(
            std::string("color " + std::to_string(i + 1)).c_str(),
            (float*)&(colors[i]), ImGuiColorEditFlags_NoInputs))
      {
        if (numInterpolatedColors == 2)
          updatePalette({imvec4ToColor(colors[0]), imvec4ToColor(colors[1])});
        if (numInterpolatedColors == 3)
          updatePalette({imvec4ToColor(colors[0]), imvec4ToColor(colors[1]),
                         imvec4ToColor(colors[2])});
        if (numInterpolatedColors == 4)
          updatePalette({imvec4ToColor(colors[0]), imvec4ToColor(colors[1]),
                         imvec4ToColor(colors[2]), imvec4ToColor(colors[3])});
        m_grid.clear();
        updateView = true;
      }
    }
    ImGui::End();
  }

  if (updateView)
  {
    updateGrid(smooth);
  }
  updateView = false;

  ImGui::Image((void*)m_view, ImVec2(m_width, m_height));

  auto mousePositionAbsolute = ImGui::GetMousePos();
  auto screenPositionAbsolute = ImGui::GetItemRectMin();

  bool hovering = ImGui::IsItemHovered();
  bool mouseDragging = ImGui::IsMouseDragging(ImGuiMouseButton_Left);

  mouseX = mousePositionAbsolute.x - screenPositionAbsolute.x;
  mouseY = mousePositionAbsolute.y - screenPositionAbsolute.y;

  double complexX = (mouseX * ((m_xmax - m_xmin) / m_width)) + m_xmin;
  double complexY = -((mouseY * ((m_ymax - m_ymin) / m_height)) + m_ymin);
  // negative because the top left corner is (0,0), not the bottom left corner

  static bool wasDragging = false;

  if (hovering && mouseDragging)
  {
    auto dragDelta = ImGui::GetMouseDragDelta();
    ImGui::ResetMouseDragDelta();
    if (dragDelta.x != 0 || dragDelta.y != 0)
    {
      auto complexDiffX = dragDelta.x * ((m_xmax - m_xmin) / m_width);
      auto complexDiffY = dragDelta.y * ((m_ymax - m_ymin) / m_height);

      m_xmin -= complexDiffX;
      m_xmax -= complexDiffX;
      m_ymin -= complexDiffY;
      m_ymax -= complexDiffY;

      updateView = true;
      wasDragging = true;
      m_grid.translate(dragDelta.x, dragDelta.y);
    }
  }
  // zoom in
  if (hovering && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !wasDragging)
  {
    m_xmin = (m_xmin + complexX) / 2;
    m_xmax = (m_xmax + complexX) / 2;

    m_ymin = (m_ymin - complexY) / 2;
    m_ymax = (m_ymax - complexY) / 2;
    m_grid.clear();
    updateView = true;
  }
  // zoom out
  if (hovering && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
  {
    m_xmin -= (complexX - m_xmin);
    m_xmax += (m_xmax - complexX);

    m_ymin += (complexY + m_ymin);
    m_ymax += (m_ymax + complexY);
    m_grid.clear();
    updateView = true;
  }

  if (ImGui::IsMouseReleased(ImGuiMouseButton_Left && wasDragging))
  {
    wasDragging = false;
  }

  if (hovering && debug)
  {
    auto result = calculatePixel(complexX, complexY, smooth);
    ImGui::Text("Screen space: x:%d, y:%d", mouseX, mouseY);
    ImGui::Text("Complex space: x:%f, y:%f", complexX, complexY);
    ImGui::Text("Iterations: %f", result);
  }
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

  
}

void Mandelbrot::loadGrid()
{
  if (m_texture)
    m_texture->Release();
  if (m_view)
    m_view->Release();

  automata::LoadTextureFromData(m_grid.getData(), &m_view, &m_texture,
                                m_pDevice, m_width, m_height);
}

void Mandelbrot::updateGrid(Smooth smooth)
{
  std::vector<std::future<bool>> futures;
  for (uint32_t i = 0; i < m_numThreads; i++)
  {
    futures.push_back(std::async(std::launch::async,
                                 &Mandelbrot::getMandelbrotPixels, this, i,
                                 m_numThreads, smooth));
  }

  for (auto&& future : futures)
  {
    auto result = future.get();
    if (!result)
      throw std::runtime_error("fractal generation failed");
  }
  loadGrid();
}

void Mandelbrot::updatePalette(std::vector<Color> colorList)
{
  if (colorList.size() < 2 || colorList.size() > 4)
  {
    throw std::runtime_error("cannot interpolate between" +
                             std::to_string(colorList.size()) + " colors");
  }

  if (colorList.size() == 2)
    m_palette.interpolate(colorList[0], colorList[1]);
  if (colorList.size() == 3)
    m_palette.interpolate(colorList[0], colorList[1], colorList[2]);
  if (colorList.size() == 4)
    m_palette.interpolate(colorList[0], colorList[1], colorList[2],
                          colorList[3]);
}

bool Mandelbrot::getMandelbrotPixels(uint32_t offset, uint32_t numWorkers, Smooth smooth)
{
  for (uint32_t x = 0; x < m_width; x++)
  {
    for (uint32_t y = offset; y < m_height; y += numWorkers)
    {
      if (!m_grid.checkCell(y, x)) // only update pixels that are empty
      {
        // translate from pixel space to our virtual space
        double v_x = m_xmin + ((m_xmax - m_xmin) / m_width) * x;
        double v_y = m_ymin + ((m_ymax - m_ymin) / m_height) * y;

        double result = calculatePixel(v_x, v_y, smooth);
        if (result == -1.0)
        {
          m_grid.setCellDirectly(y, x, Color{0, 0, 0, 255});
        }
        else
        {
          if (smooth == Smooth::Distance)
          {
            result = 1 / result;
          }
          else
          {
            result = normalizeIteration(result);
          }
          double gradient = result - (int)result;
          int step = (int)result % m_palette.numColors;
          int nextStep = (step + 1) % m_palette.numColors;
          auto baseColor = m_palette.getColor(step);
          auto nextColor = m_palette.getColor(nextStep);
          // interpolate between baseColor and nextColor
          auto newColor = Color{
            (uint8_t)(baseColor.r + (nextColor.r - baseColor.r) * gradient),
            (uint8_t)(baseColor.g + (nextColor.g - baseColor.g) * gradient),
            (uint8_t)(baseColor.b + (nextColor.b - baseColor.b) * gradient),
            255};
          m_grid.setCellDirectly(y, x, newColor);
          
        }
      }
    }
  }
  return true;
}

double Mandelbrot::calculatePixel(double x_0, double y_0, Smooth smooth)
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
         iteration < m_iterations)
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
  if (iteration == m_iterations)
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
    double mod_z = sqrt(x_2 + y_2);
    return mod_z * log(mod_z) / sqrt(dz_x * dz_x + dz_y * dz_y);
  }
  return iteration;
}
