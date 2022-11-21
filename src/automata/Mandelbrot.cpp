#include "Mandelbrot.hpp"

#include "imgui/imgui.h"
#include "utils/LoadTextureFromData.hpp"

#include <d3d11.h>
#include <complex>
#include <string>
#include <thread>
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
} // namespace

Mandelbrot::Mandelbrot(uint64_t height, uint64_t width, ID3D11Device* pDevice)
  : m_height(height),
    m_width(width),
    m_xmin(-2.0),
    m_xmax(2.0),
    m_ymin(-1.0),
    m_ymax(1.0),
    m_iterations(50),
    m_palette(100),
    m_numThreads(std::thread::hardware_concurrency()),
    m_grid(width, height),
    m_pDevice(pDevice),
    m_texture(NULL),
    m_view(NULL),
    m_updateView(true)
{
  loadGrid();
  m_palette.interpolate(Color{0, 0, 0, 255}, Color{255, 255, 255, 255});
}

void Mandelbrot::showAutomataWindow()
{
  static bool displayRuleMenu = false;

  static uint32_t mouseX;
  static uint32_t mouseY;

  if (ImGui::Button("Show Rule Editor"))
    displayRuleMenu = true;
  if (displayRuleMenu)
    showRuleMenu(displayRuleMenu);

  if (m_updateView)
    updateGrid();
  m_updateView = false;

  ImGui::Image((void*)m_view, ImVec2(m_width, m_height));

  auto mousePositionAbsolute = ImGui::GetMousePos();
  auto screenPositionAbsolute = ImGui::GetItemRectMin();

  mouseX = mousePositionAbsolute.x - screenPositionAbsolute.x;
  mouseY = mousePositionAbsolute.y - screenPositionAbsolute.y;

  double complexX = (mouseX * ((m_xmax - m_xmin) / m_width)) + m_xmin;
  double complexY = -((mouseY * ((m_ymax - m_ymin) / m_height)) + m_ymin);
  // negative because the top left corner is (0,0), not the bottom left corner

  static bool wasDragging = false;

  if (ImGui::IsItemHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
  {
    auto dragDelta = ImGui::GetMouseDragDelta();
    ImGui::ResetMouseDragDelta();

    auto complexDiffX = dragDelta.x * ((m_xmax - m_xmin) / m_width);
    auto complexDiffY = dragDelta.y * ((m_ymax - m_ymin) / m_height);

    m_xmin -= complexDiffX;
    m_xmax -= complexDiffX;
    m_ymin -= complexDiffY;
    m_ymax -= complexDiffY;

    m_updateView = true;
    wasDragging = true;
  }

  if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !wasDragging)
  {
    m_xmin = (m_xmin + complexX) / 2;
    m_xmax = (m_xmax + complexX) / 2;

    m_ymin = (m_ymin - complexY) / 2;
    m_ymax = (m_ymax - complexY) / 2;
    m_updateView = true;
  }

  if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
  {
    m_xmin -= (complexX - m_xmin) / 2;
    m_xmax += (m_xmax - complexX) / 2;

    m_ymin += (complexY + m_ymin) / 2;
    m_ymax += (m_ymax + complexY) / 2;
    m_updateView = true;
  }

  if (ImGui::IsMouseReleased(ImGuiMouseButton_Left && wasDragging))
  {
    wasDragging = false;
  }

  if (ImGui::IsItemHovered())
  {
    ImGui::Text("Complex space: %f, %f", complexX, complexY);
  }
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}

void Mandelbrot::showRuleMenu(bool& show)
{
  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoResize;
  ImGui::Begin("Fractal Options", &show, flags);

  if (ImGui::SliderInt("Iterations", &m_iterations, 0, 2000))
    m_updateView = true;
  ImGui::Text("Palette options");

  static int numInterpolatedColors = 2; //a    b     g     r
  static std::vector<ImVec4> colors = {{1.0f, 0.0f, 0.0f, 0.0f},
                                       {1.0f, 1.0f, 1.0f, 1.0f},
                                       {1.0f, 0.0f, 0.0f, 1.0f},
                                       {1.0f, 0.0f, 1.0f, 0.0f}};

  const char* items[] = {"2", "3", "4"};
  static int item_current = 0;
  if (ImGui::Combo("Number of palette colors", &item_current, items,
    IM_ARRAYSIZE(items)))
  {
    std::vector<Color> interpolateList;
    for (int i = 0; i < numInterpolatedColors; i++)
    {
      interpolateList.push_back(imvec4ToColor(colors[i]));
    }
    updatePalette(interpolateList);
    m_updateView = true;
  }
  numInterpolatedColors = item_current + 2;

  for (int i = 0; i < numInterpolatedColors; i++)
  {
    if (ImGui::ColorEdit4((std::string("Color #") + std::to_string(i)).c_str(), (float*)&colors[i],
                   ImGuiColorEditFlags_NoInputs))
    {
      std::vector<Color> interpolateList;
      for (int i = 0; i < numInterpolatedColors; i++)
      {
        interpolateList.push_back(imvec4ToColor(colors[i]));
      }
      updatePalette(interpolateList);
      m_updateView = true;
    }
  }


  ImGui::End();
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

void Mandelbrot::updateGrid()
{
  std::vector<std::future<bool>> futures;
  for (uint32_t i = 0; i < m_numThreads; i++)
  {
    futures.push_back(std::async(std::launch::async,
                                 &Mandelbrot::getMandelbrotPixels, this, i,
                                 m_numThreads));
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

bool Mandelbrot::getMandelbrotPixels(uint32_t offset, uint32_t numWorkers)
{
  for (uint32_t x = 0; x < m_width; x++)
  {
    for (uint32_t y = offset; y < m_height; y += numWorkers)
    {
      // translate from pixel space to our virtual space
      double v_x = m_xmin + ((m_xmax - m_xmin) / m_width) * x;
      double v_y = m_ymin + ((m_ymax - m_ymin) / m_height) * y;

      auto result = calculatePixel(v_x, v_y);
      if (result.gradient == -1.0)
      {
        m_grid.setCellDirectly(y, x, Color{0, 0, 0, 255});
      }
      else
      {
        auto step = result.paletteIndex % m_palette.numColors;

        auto baseColor = m_palette.getColor(step);
        auto nextColor = m_palette.getColor((step + 1) % m_palette.numColors);
        // interpolate between baseColor and nextColor

        auto newColor = Color{
          (uint8_t)(baseColor.red +
                    (nextColor.red - baseColor.red) * result.gradient),
          (uint8_t)(baseColor.green +
                    (nextColor.green - baseColor.green) * result.gradient),
          (uint8_t)(baseColor.blue +
                    (nextColor.blue - baseColor.blue) * result.gradient),
          255};

        m_grid.setCellDirectly(y, x, newColor);
      }
    }
  }
  return true;
}

MandelbrotResult Mandelbrot::calculatePixel(double x, double y)
{
  std::complex<double> z = {0, 0};
  std::complex<double> before = {0, 0};
  std::complex<double> c = {x, y};

  uint32_t iteration = 0;
  while (z.imag() * z.imag() + z.real() * z.real() < 4 &&
         iteration < m_iterations)
    
  {
    before = z;
    z = (z * z) + c;
    iteration++;
  }
  if (iteration == m_iterations)
  {
    return MandelbrotResult{0, -1.0f}; // inside the mandelbrot set
  }
  auto min = sqrt(before.imag() * before.imag() + before.real() * before.real());
  auto max = sqrt(z.imag() * z.imag() + z.real() * z.real());

  auto gradient = (2 - min) / (max - min);
  return MandelbrotResult{iteration, gradient};
}
