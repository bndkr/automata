#include "Mandelbrot.hpp"

namespace mandelbrot
{
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

