#include "Palette.hpp"

Color Palette::getColor(uint64_t index)
{
  if (index >= numColors)
  {
    throw std::runtime_error("color index out of range");
  }
  return Color{buffer.get(index * 3), buffer.get(index * 3 + 1),
               buffer.get(index * 3 + 2), 255};
}

void Palette::setColor(Color color, uint64_t index)
{
  if (index >= numColors)
  {
    throw std::runtime_error("color index out of range");
  }
  buffer.set(index * 3, color.red);
  buffer.set(index * 3 + 1, color.blue);
  buffer.set(index * 3 + 2, color.green);
}

void Palette::interpolate(Color color1, Color color2)
{
  double half = numColors / 2;
  
  uint64_t i = 0;
  for (i; i < half; i++)
  {
    buffer.set(i * 3,  color1.blue * ((half - i) / half) + color2.blue * (i / half));
    buffer.set(i * 3 + 1,  color1.green * ((half - i) / half) + color2.green * (i / half));
    buffer.set(i * 3 + 2,  color1.red * ((half - i) / half) + color2.red * (i / half));

    buffer.set(i * 3 + (numColors * 3) / 2, 
      color2.blue * ((half - i) / half) + color1.blue * (i / half));
    buffer.set(i * 3 + 1 + (numColors * 3) / 2, 
      color2.green * ((half - i) / half) + color1.green * (i / half));
    buffer.set(i * 3 + 2 + (numColors * 3) / 2, 
      color2.red * ((half - i) / half) + color1.red * (i / half));
  }
}

void Palette::interpolate(Color color1, Color color2, Color color3)
{
  double third = numColors / 3;

  uint64_t i = 0;
  for (i; i < third; i++)
  {
    buffer.set(i * 3,  color1.blue * ((third - i) / third) + color2.blue * (i / third));
    buffer.set(i * 3 + 1,  color1.green * ((third - i) / third) + color2.green * (i / third));
    buffer.set(i * 3 + 2,  color1.red * ((third - i) / third) + color2.red * (i / third));

    buffer.set(i * 3 + (numColors * 3) / 3, 
      color2.blue * ((third - i) / third) + color3.blue * (i / third));
    buffer.set(i * 3 + 1 + (numColors * 3) / 3, 
      color2.green * ((third - i) / third) + color3.green * (i / third));
    buffer.set(i * 3 + 2 + (numColors * 3) / 3, 
      color2.red * ((third - i) / third) + color3.red * (i / third));

    buffer.set(i * 3 + (numColors * 6) / 3, 
      color3.blue * ((third - i) / third) + color1.blue * (i / third));
    buffer.set(i * 3 + 1 + (numColors * 6) / 3, 
      color3.green * ((third - i) / third) + color1.green * (i / third));
    buffer.set(i * 3 + 2 + (numColors * 6) / 3, 
      color3.red * ((third - i) / third) + color1.red * (i / third));
  }
}

void Palette::interpolate(Color color1, Color color2, Color color3,
                          Color color4)
{
  double fourth = numColors / 4;

  uint64_t i = 0;
  for (i; i < fourth; i++)
  {
    buffer.set(i * 3, color1.blue * ((fourth - i) / fourth) + color2.blue * (i / fourth));
    buffer.set(i * 3 + 1,
               color1.green * ((fourth - i) / fourth) + color2.green * (i / fourth));
    buffer.set(i * 3 + 2, color1.red * ((fourth - i) / fourth) + color2.red * (i / fourth));

    buffer.set(i * 3 + (numColors * 3) / 4, 
      color2.blue * ((fourth - i) / fourth) + color3.blue * (i / fourth));
    buffer.set(i * 3 + 1 + (numColors * 3) / 4, 
      color2.green * ((fourth - i) / fourth) + color3.green * (i / fourth));
    buffer.set(i * 3 + 2 + (numColors * 3) / 4, 
      color2.red * ((fourth - i) / fourth) + color3.red * (i / fourth));

    buffer.set(i * 3 + (numColors * 6) / 4, 
      color3.blue * ((fourth - i) / fourth) + color4.blue * (i / fourth));
    buffer.set(i * 3 + 1 + (numColors * 6) / 4, 
      color3.green * ((fourth - i) / fourth) + color4.green * (i / fourth));
    buffer.set(i * 3 + 2 + (numColors * 6) / 4, 
      color3.red * ((fourth - i) / fourth) + color4.red * (i / fourth));

    buffer.set(i * 3 + (numColors * 9) / 4, 
      color4.blue * ((fourth - i) / fourth) + color1.blue * (i / fourth));
    buffer.set(i * 3 + 1 + (numColors * 9) / 4, 
      color4.green * ((fourth - i) / fourth) + color1.green * (i / fourth));
    buffer.set(i * 3 + 2 + (numColors * 9) / 4, 
      color4.red * ((fourth - i) / fourth) + color1.red * (i / fourth));
  }
}