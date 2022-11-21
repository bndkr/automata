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
  buffer.set(index * 3, color.r);
  buffer.set(index * 3 + 1, color.b);
  buffer.set(index * 3 + 2, color.g);
}

void Palette::interpolate(Color color1, Color color2)
{
  double half = numColors / 2;
  
  uint64_t i = 0;
  for (i; i < half; i++)
  {
    buffer.set(i * 3,  color1.b * ((half - i) / half) + color2.b * (i / half));
    buffer.set(i * 3 + 1,  color1.g * ((half - i) / half) + color2.g * (i / half));
    buffer.set(i * 3 + 2,  color1.r * ((half - i) / half) + color2.r * (i / half));

    buffer.set(i * 3 + (numColors * 3) / 2, 
      color2.b * ((half - i) / half) + color1.b * (i / half));
    buffer.set(i * 3 + 1 + (numColors * 3) / 2, 
      color2.g * ((half - i) / half) + color1.g * (i / half));
    buffer.set(i * 3 + 2 + (numColors * 3) / 2, 
      color2.r * ((half - i) / half) + color1.r * (i / half));
  }
}

void Palette::interpolate(Color color1, Color color2, Color color3)
{
  double third = numColors / 3;

  uint64_t i = 0;
  for (i; i < third; i++)
  {
    buffer.set(i * 3,  color1.b * ((third - i) / third) + color2.b * (i / third));
    buffer.set(i * 3 + 1,  color1.g * ((third - i) / third) + color2.g * (i / third));
    buffer.set(i * 3 + 2,  color1.r * ((third - i) / third) + color2.r * (i / third));

    buffer.set(i * 3 + (numColors * 3) / 3, 
      color2.b * ((third - i) / third) + color3.b * (i / third));
    buffer.set(i * 3 + 1 + (numColors * 3) / 3, 
      color2.g * ((third - i) / third) + color3.g * (i / third));
    buffer.set(i * 3 + 2 + (numColors * 3) / 3, 
      color2.r * ((third - i) / third) + color3.r * (i / third));

    buffer.set(i * 3 + (numColors * 6) / 3, 
      color3.b * ((third - i) / third) + color1.b * (i / third));
    buffer.set(i * 3 + 1 + (numColors * 6) / 3, 
      color3.g * ((third - i) / third) + color1.g * (i / third));
    buffer.set(i * 3 + 2 + (numColors * 6) / 3, 
      color3.r * ((third - i) / third) + color1.r * (i / third));
  }
}

void Palette::interpolate(Color color1, Color color2, Color color3,
                          Color color4)
{
  double fourth = numColors / 4;

  uint64_t i = 0;
  for (i; i < fourth; i++)
  {
    buffer.set(i * 3, color1.b * ((fourth - i) / fourth) + color2.b * (i / fourth));
    buffer.set(i * 3 + 1,
               color1.g * ((fourth - i) / fourth) + color2.g * (i / fourth));
    buffer.set(i * 3 + 2, color1.r * ((fourth - i) / fourth) + color2.r * (i / fourth));

    buffer.set(i * 3 + (numColors * 3) / 4, 
      color2.b * ((fourth - i) / fourth) + color3.b * (i / fourth));
    buffer.set(i * 3 + 1 + (numColors * 3) / 4, 
      color2.g * ((fourth - i) / fourth) + color3.g * (i / fourth));
    buffer.set(i * 3 + 2 + (numColors * 3) / 4, 
      color2.r * ((fourth - i) / fourth) + color3.r * (i / fourth));

    buffer.set(i * 3 + (numColors * 6) / 4, 
      color3.b * ((fourth - i) / fourth) + color4.b * (i / fourth));
    buffer.set(i * 3 + 1 + (numColors * 6) / 4, 
      color3.g * ((fourth - i) / fourth) + color4.g * (i / fourth));
    buffer.set(i * 3 + 2 + (numColors * 6) / 4, 
      color3.r * ((fourth - i) / fourth) + color4.r * (i / fourth));

    buffer.set(i * 3 + (numColors * 9) / 4, 
      color4.b * ((fourth - i) / fourth) + color1.b * (i / fourth));
    buffer.set(i * 3 + 1 + (numColors * 9) / 4, 
      color4.g * ((fourth - i) / fourth) + color1.g * (i / fourth));
    buffer.set(i * 3 + 2 + (numColors * 9) / 4, 
      color4.r * ((fourth - i) / fourth) + color1.r * (i / fourth));
  }
}