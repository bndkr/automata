#include "Grid.hpp"

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

void Grid::clear()
{
  m_data.fill(0);
  m_changes.clear();
}

void Grid::applyChanges()
{
  for (const auto& change : m_changes)
  {
    m_data.set((change.row * m_width + change.col) * 4, change.color.red);
    m_data.set((change.row * m_width + change.col) * 4 + 1, change.color.green);
    m_data.set((change.row * m_width + change.col) * 4 + 2, change.color.blue);
    m_data.set((change.row * m_width + change.col) * 4 + 3, change.color.alpha);
  }
  m_changes.clear();
}
