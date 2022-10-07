#ifndef AUTOMATA_GRID
#define AUTOMATA_GRID

#include <cstdint>
#include <vector>

struct Color {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t alpha;
};

struct CellChange
{
  uint64_t row;
  uint64_t col;
  Color color;
};

class Grid {
public:
  Grid() : m_data(), m_changes(), m_width(0), m_height(0) {}

  Grid(uint64_t width, uint64_t height) : m_width(width), m_height(height) {
    // fill grid with empty cells
    uint64_t size = width * height * 4;
    m_data.reserve(size);
    for (uint64_t i = 0; i < size; i++) {
      m_data.push_back(0);
    }
  }

  uint8_t *getData() { return m_data.data(); }

  uint64_t getHeight() { return m_height; }
  uint64_t getWidth() { return m_width; }

  Color getCell(uint64_t row, uint64_t col);

  bool setCell(uint64_t row, uint64_t col, Color color);

  bool checkCell(uint64_t row, uint64_t col);

  void fill(Color color) {
    for (uint64_t i = 0; i < m_data.size(); i += 4) {
      m_data[i] = color.red;
      m_data[i + 1] = color.green;
      m_data[i + 2] = color.blue;
      m_data[i + 3] = color.alpha;
    }
  }

  void clear();

  void applyChanges();

  std::vector<uint8_t> &getArray() { return m_data; }

private:
  std::vector<uint8_t> m_data; // for reading
  std::vector<CellChange> m_changes; // for writing
  uint64_t m_width;
  uint64_t m_height;
};

#endif
