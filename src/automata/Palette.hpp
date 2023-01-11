#ifndef AUTOMATA_PALETTE
#define AUTOMATA_PALETTE

#include "Grid.hpp"
#include "imgui/imgui.h"

struct Palette
{
  Palette(std::vector<Color> colors, uint32_t numSteps = 100)
    : m_colors(colors), m_numSteps(numSteps)
  {
  }

  Color getColor(double index);

  void updateColors(std::vector<ImVec4> colors);
  void updateSize(uint32_t numSteps)
  {
    m_numSteps = numSteps;
  }

  uint32_t getNumSteps()
  {
    return m_numSteps;
  }

  std::vector<Color> m_colors;
  uint32_t m_numSteps;
};
#endif
