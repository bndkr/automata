#include "Palette.hpp"

namespace
{
double fmod(double f, uint32_t n)
{
  for (;;)
  { // could be infinite if f < 0
    if (f - n >= 0)
      f -= n;
    else
      return f;
  }
}

  // assumed that 'interval' is between 0 and 1
Color interpolate(const Color& color1, const Color& color2, const float interval)
{
  return Color{(uint8_t)((float) color2.r * interval + (float) (1 - interval) * color1.r),
               (uint8_t)((float) color2.g * interval + (float) (1 - interval) * color1.g),
               (uint8_t)((float) color2.b * interval + (float) (1 - interval) * color1.b),
               (uint8_t)((float) color2.a * interval + (float) (1 - interval) * color1.a)};
}

Color imvec4ToColor(ImVec4 vec)
{
  uint8_t red = vec.x * 255;
  uint8_t green = vec.y * 255;
  uint8_t blue = vec.z * 255;
  uint8_t alpha = vec.w * 255;
  return Color{red, green, blue, alpha};
} 
}

// index is assumed to be (numIterations + gradient)
Color Palette::getColor(double index)
{
  index = fmod(index, m_numSteps);
  uint32_t numColors = m_colors.size();
  uint32_t stepSize = m_numSteps / numColors;
  uint32_t colorIndex1 = index / stepSize;
  return interpolate(m_colors[colorIndex1],
                     m_colors[(colorIndex1 + 1) % numColors],
                     fmod(index, stepSize) / stepSize);
}

void Palette::updateColors(std::vector<ImVec4> colors)
{
  m_colors.clear();
  for (const auto& color : colors)
  {
    m_colors.push_back(imvec4ToColor(color));
  }
}
