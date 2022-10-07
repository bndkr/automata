#include "Grid.hpp"

Color Grid::getCell(uint64_t row, uint64_t col) {
  uint64_t cellIndex = row * m_width + col;
  return Color{m_data[cellIndex], m_data[cellIndex+1], m_data[cellIndex+2],
               m_data[cellIndex+3]};
}

bool Grid::setCell(uint64_t row, uint64_t col, Color color)
{
  if (row >= m_height || col >= m_width || !m_data.data())
  {
    return false;
  }
  m_changes.push_back(CellChange{ row, col, color });
  
  return true;
}

bool Grid::checkCell(uint64_t row, uint64_t col)
{
  // check alpha band
  return m_data[(row * m_width + col) * 4 + 3] != 0;
}

void Grid::clear()
{
  std::memset(m_data.data(), 0, m_data.size());
  m_changes.clear();
}

void Grid::applyChanges()
{
  for (const auto& change : m_changes)
  {
    m_data[(change.row * m_width + change.col) * 4] = change.color.red;
    m_data[(change.row * m_width + change.col) * 4 + 1] = change.color.green;
    m_data[(change.row * m_width + change.col) * 4 + 2] = change.color.blue;
    m_data[(change.row * m_width + change.col) * 4 + 3] = change.color.alpha;
  }
  m_changes.clear();
}
