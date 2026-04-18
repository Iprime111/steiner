#pragma once

#include "graph.hpp"
#include "types.hpp"

#include <cstdlib>
#include <limits>
#include <ranges>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace steiner {
struct ManhattanMetric final {
    static constexpr Distance calculate(const Point& a, const Point& b) {
        return abs_internal(a.x - b.x) + abs_internal(a.y - b.y);
    }
    
  private:
    // std::abs is not constexpr in msvc
    static constexpr Distance abs_internal(Distance val) {
        return val < 0 ? -val : val;
    }
};

class HananGrid final {
  public:
    explicit HananGrid(std::ranges::input_range auto&& points) {
        for (auto&& point : points) {
            x_coord_.push_back(point.x);
            y_coord_.push_back(point.y);
            occupied_points_.insert(point);
        }
    }

    auto get_candidates() const {
        // std::views::cartesian_product is still unimplemented
        std::vector<Point> candidates;

        for (auto x : x_coord_) {
            for (auto y : y_coord_) {
                Point p{x, y};

                if (is_occupied(p)) {
                    continue;
                }

                candidates.push_back(p);
            }
        }

        return candidates;
    }

    void occupy(Point p) { occupied_points_.insert(p); }
    bool is_occupied(Point p) const { return occupied_points_.contains(p); }

  private:
    std::vector<Distance> x_coord_;
    std::vector<Distance> y_coord_;
    std::unordered_set<Point> occupied_points_;
};

class MSTSolver final {
    struct MstNodeInfo {
        Distance min_dist{std::numeric_limits<Distance>::max()};
        NodeId parent{DefaultGraph::kInvalidNodeId};
        bool visited{false};
    };

  public:
    MSTSolver(DefaultGraph& graph) : graph_(graph) {}

    Distance compute();

  private:
    NodeId find_min_dist_node();
    void update_node_info(NodeId min_dist_node);

    std::unordered_map<NodeId, MstNodeInfo> node_info_;
    DefaultGraph& graph_;
};

class Basic1SteinerAlgo {
  public:
    Basic1SteinerAlgo(DefaultGraph& graph) : graph_(graph) {}
    virtual ~Basic1SteinerAlgo() = default;
    
    Distance compute();

  protected:
    Distance compute_cost_with_candidate(Point coord, MSTSolver& mst) ;
    void remove_bad_stenier_points();

  private:
    std::pair<Point, bool> choose_candidate(const HananGrid& grid, MSTSolver& mst);
    
  protected:
    DefaultGraph& graph_;
};

class Batched1SteinerAlgo final : protected Basic1SteinerAlgo {
  using Base = Basic1SteinerAlgo;
  
  public:
    Batched1SteinerAlgo(DefaultGraph& graph) : Base(graph) {}
    virtual ~Batched1SteinerAlgo() override = default;
    
    Distance compute();
    
  private:
    std::vector<Point> generate_batch(const HananGrid& grid, MSTSolver& mst);
    std::vector<NodeId> get_affected(NodeId candidate);
    
    Graph<Point, DefaultEdgeData> conflict_graph_;
};

class GraphVerifier final {
  public:
    struct Review {
        bool passed{};
        std::vector<std::pair<NodeId, std::string>> bad_nodes;
        std::string comment;
    };

    GraphVerifier(const DefaultGraph& graph) : graph_(graph) {}
    Review verify();

  private:
    const DefaultGraph& graph_;
};
}  // namespace steiner

template <>
struct fmt::formatter<steiner::GraphVerifier::Review> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    auto format(const steiner::GraphVerifier::Review& r, format_context& ctx) const {
        if (r.passed) {
            return fmt::format_to(ctx.out(), "Graph check passed");
        }

        auto out = fmt::format_to(ctx.out(), "Graph check failed\n");

        out = fmt::format_to(out, "Comment: {}\n", r.comment.empty() ? "No comment" : r.comment);

        if (!r.bad_nodes.empty()) {
            out = fmt::format_to(out, "Node-specific issues:");
            for (const auto& [id, error] : r.bad_nodes) {
                out = fmt::format_to(out, "\n  - Node [{}]: {}", id, error);
            }
        }

        return out;
    }
};
