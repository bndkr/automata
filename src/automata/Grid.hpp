#ifndef AUTOMATA_GRID
#define AUTOMATA_GRID

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

struct Buffer
{
  std::vector<uint8_t> arr;
  uint64_t len;

  Buffer(uint64_t size) : len(size)
  {
    for (uint32_t i = 0; i < size; i++)
    {
      arr.push_back(0);
    }
  }
  ~Buffer()
  {
    // TODO: this needs to free memory, but delete[] craps out
  }
  uint8_t get(uint64_t index)
  {
    if (index < len)
      return *(arr.data() + index);
    else
      throw std::runtime_error("buffer access out of bounds");
  }

  void set(uint64_t index, uint8_t value)
  {
    if (index < len)
      *(arr.data() + index) = value;
    else
      throw std::runtime_error("buffer access out of bounds");
  }

  void fill(uint8_t value)
  {
    std::memset(arr.data(), value, len);
  }
};

struct Color
{
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

class Grid
{
public:
  Grid(uint64_t width, uint64_t height)
    : m_width(width), m_height(height), m_data(width * height * 4)
  {
    
    m_data.fill(0);
  }

  uint8_t* getData()
  {
    return m_data.arr.data();
  }

  uint64_t getHeight()
  {
    return m_height;
  }
  uint64_t getWidth()
  {
    return m_width;
  }

  Color getCell(uint64_t row, uint64_t col);

  bool setCell(uint64_t row, uint64_t col, Color color);

  bool checkCell(uint64_t row, uint64_t col);

  void fill(Color color) // needs to be tested
  {
    uint32_t c = color.red + (uint32_t)((uint32_t)color.green << 8) +
                 ((uint32_t)color.blue << 16) + ((uint32_t)color.alpha << 24);

    for (uint64_t i = 0; i < m_data.len; i += 4)
    {
      std::memcpy(m_data.arr.data() + i, &c, 4);
    }
  }

  void clear();

  void applyChanges();

  Buffer m_data;
private:
  std::vector<CellChange> m_changes; // for writing
  uint64_t m_width;
  uint64_t m_height;
};

void upsampleGrid(Grid& unit, Grid& scaled, uint32_t scale);

#endif
