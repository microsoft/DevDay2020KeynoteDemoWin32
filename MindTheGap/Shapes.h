#pragma once
#include <vector>

enum class ShapeKind
{
  Circle,
  Square,
};

struct Shape
{
  ShapeKind kind{};
  POINTS center{};
  UINT size{};

  short Left() const { return center.x - (size / 2); }
  short Right() const { return  Left() + size; }
  short Top() const { return  center.y - (size / 2); }
  short Bottom() const { return  Top() + size; }

  const RECT Rect() const { return RECT{ Left(), Top(), Right(), Bottom() }; }
};

class ShapeDrawing
{
  std::vector<Shape> shapes{};

public:
  void AddShape(const Shape& shape)
  {
    shapes.push_back(shape);
  }
  
  const std::vector<Shape>& GetAllShapes() const
  {
    return shapes;
  }
};

