#include "algo.hpp"
#include "fmt/base.h"
#include "fmt/format.h"
#include "types.hpp"

#include <ranges>
#include <utility>
#include <vector>

namespace steiner {
Distance MSTSolver::compute() {
    if (graph_.nodes_count() == 0) {
        return 0;
    }

    if (graph_.edges_count() != 0) {
        LOG_CRITICAL("MST solver works only for graphs without any edges");
    }

    node_info_.clear();
    Distance total_cost = 0;

    auto nodes = graph_.node_ids();
    node_info_[nodes.front()].min_dist = 0;

    for (auto id : nodes) {
        auto min_dist_node = find_min_dist_node();
        node_info_[min_dist_node].visited = true;

        if (node_info_[min_dist_node].parent != DefaultGraph::kInvalidNodeId) {
            auto begin = node_info_[min_dist_node].parent;
            auto end = min_dist_node;

            graph_.add_edge({}, begin, end);
            total_cost +=
                ManhattanMetric::calculate(graph_.node_data(begin).coord, graph_.node_data(end).coord);
        }

        update_node_info(min_dist_node);
    }

    return total_cost;
}

NodeId MSTSolver::find_min_dist_node() {
    NodeId min_dist_node = DefaultGraph::kInvalidNodeId;

    // TODO Could possibly have smth like "std::set<NodeId> unvisited" to speed up this cycle
    for (auto id :
         graph_.node_ids() | std::views::filter([&](auto node_id) { return !node_info_[node_id].visited; })) {
        if ((min_dist_node == DefaultGraph::kInvalidNodeId) ||
            (node_info_[id].min_dist < node_info_[min_dist_node].min_dist)) {
            min_dist_node = id;
        }
    }

    if (min_dist_node == DefaultGraph::kInvalidNodeId ||
        node_info_[min_dist_node].min_dist == std::numeric_limits<Distance>::max()) {
        LOG_CRITICAL("MST error. Unable to find node with a minimal distance to the graph.");
    }

    return min_dist_node;
}

void MSTSolver::update_node_info(NodeId min_dist_node) {
    for (auto id :
         graph_.node_ids() | std::views::filter([&](auto node_id) { return !node_info_[node_id].visited; })) {
        auto distance =
            ManhattanMetric::calculate(graph_.node_data(id).coord, graph_.node_data(min_dist_node).coord);
        if (distance < node_info_[id].min_dist) {
            node_info_[id] = {
                .min_dist = distance,
                .parent = min_dist_node,
                .visited = false,
            };
        }
    }
}

void Basic1SteinerAlgo::compute() {
    MSTSolver mst{graph_};

    auto points = graph_.node_data() | std::views::transform([](auto&& data) { return data.coord; });
    HananGrid grid{points};

    while (true) {
        auto [candidate, has_improvement] = choose_candidate(grid, mst);

        if (!has_improvement) {
            break;
        }

        graph_.add_node({
            .coord = candidate,
            .type = NodeType::kSteinerPoint,
        });
        grid.occupy(candidate);

        graph_.clear_edges();
        mst.compute();

        // remove_bad_stenier_points();
    }

    graph_.clear_edges();
    mst.compute();
}

std::pair<Point, bool> Basic1SteinerAlgo::choose_candidate(const HananGrid& grid, MSTSolver& mst) {
    Point best_x;
    Distance max_delta = 0;

    graph_.clear_edges();
    auto current_cost = mst.compute();

    for (const auto& coord : grid.get_candidates()) {
        auto candidate_id = graph_.add_node({
            .coord = coord,
            .type = NodeType::kSteinerPoint,
        });

        graph_.clear_edges();
        auto new_cost = mst.compute();
        auto delta = current_cost - new_cost;

        if (delta > max_delta) {
            max_delta = delta;
            best_x = coord;
        }

        graph_.remove_node(candidate_id);
    }

    return std::make_pair(best_x, max_delta > 0);
}

void Basic1SteinerAlgo::remove_bad_stenier_points() {
    std::vector<NodeId> remove_list;

    for (auto id : graph_.node_ids() | std::views::filter([&](auto node_id) {
                       return graph_.node_data(node_id).type == NodeType::kSteinerPoint;
                   })) {
        if (graph_.node_edges(id).size() <= 2) {
            remove_list.push_back(id);
        }
    }

    for (auto id : remove_list) {
        graph_.remove_node(id);
    }
}

GraphVerifier::Review GraphVerifier::verify() {
    Review review{
        .passed = true,
    };

    for (auto id : graph_.node_ids()) {
        auto edges_count = graph_.node_edges(id).size();

        if ((graph_.node_data(id).type == NodeType::kSteinerPoint) && (edges_count <= 2)) {
            review.bad_nodes.emplace_back(id, fmt::format("Steiner node has {} < 3 edges", edges_count));
            review.passed = false;
        }
        
        if (edges_count == 0) {
            review.bad_nodes.emplace_back(id, "Detached node");
            review.passed = false;
        }
    }
    
    auto nodes_count = graph_.nodes_count();
    auto edges_count = graph_.edges_count();
    
    if ((nodes_count != 0) && (edges_count + 1 != nodes_count)) {
        review.comment = fmt::format("Expected {} edges, got {}", nodes_count - 1, edges_count);
        review.passed = false;
    }
    
    return review;
}
}  // namespace steiner
