#include "Grid.hpp"

bool Grid::setCellDirectly(uint64_t row, uint64_t col, Color color)
{
  if (row >= m_height || col >= m_width || !m_data.arr.data())
  {
    return false;
  }
  m_data.set((row * m_width + col) * 4, color.r);
  m_data.set((row * m_width + col) * 4 + 1, color.g);
  m_data.set((row * m_width + col) * 4 + 2, color.b);
  m_data.set((row * m_width + col) * 4 + 3, color.a);
  return true;
}

Color Grid::getCell(uint64_t row, uint64_t col)
{
  uint64_t cellIndex = (row * m_width + col) * 4;
  return Color{m_data.get(cellIndex), m_data.get(cellIndex + 1),
               m_data.get(cellIndex + 2), m_data.get(cellIndex + 3)};
}

bool Grid::setCell(uint64_t row, uint64_t col, Color color)
{
  if (row >= m_height || col >= m_width || !m_data.arr.data())
  {
    return false;
  }
  m_changes.push_back(CellChange{row, col, color});

  return true;
}

bool Grid::checkCell(uint64_t row, uint64_t col)
{
  // check alpha band
  return m_data.get((row * m_width + col) * 4 + 3) != 0;
}

void Grid::translate(int dx, int dy)
{
  if (dx != 0 || dy != 0)
  {
    Grid newGrid(m_width, m_height);
    for (uint32_t x = 0; x < m_width; x++)
    {
      for (uint32_t y = 0; y < m_height; y++)
      {
        Color c = getCell(y, x);

        uint32_t newX = x + dx;
        uint32_t newY = y + dy;
        if (!(newX >= m_width || newX < 0 || newY >= m_height || newY < 0))
        {
          newGrid.setCellDirectly(newY, newX, c);
        }
      }
    }
    std::memcpy(getData(), newGrid.getData(), m_height * m_width * 4);
  }
}

void Grid::clear()
{
  m_data.fill(0);
  m_changes.clear();
}

void Grid::applyChanges()
{
  for (const auto& change : m_changes)
  {
    m_data.set((change.row * m_width + change.col) * 4, change.color.r);
    m_data.set((change.row * m_width + change.col) * 4 + 1, change.color.g);
    m_data.set((change.row * m_width + change.col) * 4 + 2, change.color.b);
    m_data.set((change.row * m_width + change.col) * 4 + 3, change.color.a);
  }
  m_changes.clear();
}

void upsampleGrid(Grid& unit, Grid& scaled, uint32_t scale)
{
  uint32_t unitWidth = unit.getWidth();
  uint32_t unitHeight = unit.getHeight();
  uint32_t stride = unit.getWidth() * 4 * scale;

  for (uint64_t h = 0; h < unitHeight; h++)
  {
    for (uint64_t w = 0; w < unitWidth; w++)
    {
      Color color = unit.getCell(h, w);
      uint32_t c = color.r | ((uint32_t)color.g << 8) |
                   ((uint32_t)color.b << 16) | ((uint32_t)color.a << 24);

      uint32_t offset = (h * stride * scale) + (w * scale * 4);
      for (uint64_t sy = 0; sy < scale; sy++)
      {
        for (uint64_t sx = 0; sx < scale; sx++)
        {
          std::memcpy(scaled.getData() + offset +
                        (stride * sy) + (sx * 4),
                      &c, 4);
        }
      }
    }
  }
}
