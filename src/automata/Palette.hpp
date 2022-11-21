#ifndef AUTOMATA_PALETTE
#define AUTOMATA_PALETTE

#include "Grid.hpp"
struct Palette
{
  Palette(uint64_t numColors)
    : buffer(numColors * 3), numColors(numColors)
  {
    interpolate(Color{0, 0, 0, 255}, Color{255, 255, 255, 255});
  }

  Color getColor(uint64_t index);

  void setColor(Color color, uint64_t index);

  void interpolate(Color color1, Color color2);
  void interpolate(Color color1, Color color2, Color color3);
  void interpolate(Color color1, Color color2, Color color3, Color color4);

  Buffer buffer;
  uint32_t numColors;
};
#endif