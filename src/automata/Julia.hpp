#ifndef AUTOMATA_JULIA
#define AUTOMATA_JULIA

#include "Grid.hpp"
#include "Palette.hpp"
#include "Fractal.hpp"

#include <d3d11.h>

#include <imgui/imgui.h>
namespace julia
{
double calculatePixel(const double x_0, const double y_0, const Smooth smooth,
                      const uint32_t maxIterations, const double seedX,
                      const double seedY);
};

#endif
