#pragma once

#include "types.hpp"

#include <concepts>
#include <cstdlib>
#include <ranges>
#include <unordered_set>
#include <vector>

namespace steiner {
template <typename T>
concept DistanceMetric = requires (const Point& a, const Point& b) {
    { T::calculate(a, b) } -> std::convertible_to<Distance>;
};

struct ManhattanMetric final {
    static constexpr Distance calculate(const Point& a, const Point& b) {
        return std::abs(a.x - b.x) + std::abs(a.y - b.y);
    }
};

class HananGrid final {
  public:
    explicit HananGrid(const std::vector<Point>& terminals);
    
    auto get_candidates() const {
        std::vector<Point> candidates;
        return std::views::zip(x_coord_, y_coord_) | std::views::filter([&](auto&& point) {
            return !is_occupied(point);
        }) | std::views::transform([](auto&& point) {
            return Point{point};
        });
    }
    
    void occupy(Point p) { occupied_points_.insert(p); }
    bool is_occupied(Point p) const { return occupied_points_.contains(p); }
    
  private: 
    std::unordered_set<Distance> x_coord_;
    std::unordered_set<Distance> y_coord_;
    std::unordered_set<Point> occupied_points_;
};

} // namespace steiner