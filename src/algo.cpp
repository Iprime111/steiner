#include "algo.hpp"
#include "types.hpp"

#include <vector>

namespace steiner {
HananGrid::HananGrid(const std::vector<Point>& terminals) {
  for (auto&& point : terminals) {
      x_coord_.insert(point.x);
      y_coord_.insert(point.y);
      occupied_points_.insert(point);
  }
}
} // namepace steiner