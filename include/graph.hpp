#pragma once

#include "fmt/base.h"
#include "log.hpp"
#include "types.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <algorithm>
#include <iterator>
#include <limits>
#include <nlohmann/json.hpp>
#include <ranges>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace steiner {
// TODO add move_constructible/copy_constructible restrictions for NodeData/EdgeData
template <typename NodeData, typename EdgeData>
class Graph final {
    // TODO Use vector instead of unordered_set for small node count?
    struct Node final {
        std::unordered_set<EdgeId> edges;
    };

    struct Edge final {
        NodeId begin;
        NodeId end;
    };

  public:
    using NodeDataType = NodeData;
    using EdgeDataType = EdgeData;

    static constexpr NodeId kInvalidNodeId = -1;

    Graph() : last_edge_id_(0), last_node_id_(0) {}

    // TODO pass better type
    Graph(std::unordered_map<NodeId, NodeData>&& nodes) {
        nodes_ = std::move(nodes) | std::views::transform([&](auto&& val) {
                     auto&& [id, data] = val;
                     last_node_id_ = std::max(last_node_id_, id);
                     return std::make_pair(id, std::make_pair(Node{}, data));
                 }) |
                 std::ranges::to<std::map>();

        last_edge_id_ = 0;
    }

    const auto& node_data(NodeId id) const { return nodes_.at(id).second; }
    auto node_data() const { return nodes_ | std::views::values | std::views::elements<1>; }

    const auto& node_edges(NodeId id) const { return nodes_.at(id).first.edges; }
    auto node_edges() const {
        return nodes_ | std::views::values | std::views::transform([](auto&& node_pair) {
            return node_pair.first.edges;
        });
    }

    const auto& edge_data(EdgeId id) const { return edges_.at(id).second; }
    auto edge_data() const { return edges_ | std::views::values | std::views::elements<1>; }
    auto edge_nodes(EdgeId id) const {
        const auto& edge = edges_.at(id).first;
        return std::make_pair(edge.begin, edge.end);
    }

    auto node_ids() const { return std::views::keys(nodes_); }
    auto edge_ids() const { return std::views::keys(edges_); }
    auto nodes_count() const { return nodes_.size(); }
    auto edges_count() const { return edges_.size(); }

    void clear_edges() {
        edges_.clear();

        for (auto&& [_, node] : nodes_) {
            node.first.edges.clear();
        }
    }

    void clear() {
        nodes_.clear();
        edges_.clear();
    }

    NodeId add_node(NodeData&& node, NodeId id_hint = 0) {
        update_last_id(last_node_id_, id_hint);
        nodes_.emplace(last_node_id_, std::make_pair(Node{}, std::move(node)));
        return last_node_id_;
    }
    
    void remove_node(NodeId id) {
        auto node_it = nodes_.find(id);
 
        if (node_it == std::end(nodes_)) {
            throw std::out_of_range{fmt::format("Node {} not found", id)};
        }
        
        
        for (auto&& edge : node_it->second.first.edges) {
            NodeId other = kInvalidNodeId;
            
            auto edgeStruct = edges_[edge].first;
            
            if (edgeStruct.begin == id) {
                other = edgeStruct.end;
            } else {
                other = edgeStruct.begin;
            }
            
            nodes_[other].first.edges.erase(edge);
            edges_.erase(edge);
        }
        
        nodes_.erase(node_it);
        
        if (last_node_id_ == id) {
            --last_node_id_;
        }
    }

    EdgeId add_edge(EdgeData edge, NodeId begin, NodeId end, EdgeId id_hint = 0) {
        if (!nodes_.contains(begin) || !nodes_.contains(end)) {
            throw std::out_of_range{
                std::format("Unable to create edge {0}-{1}. Node {0} does not exist.", begin, end)};
        }

        update_last_id(last_edge_id_, id_hint);
        edges_.emplace(last_edge_id_, std::make_pair(Edge{begin, end}, std::move(edge)));
        
        nodes_[begin].first.edges.insert(last_edge_id_);
        nodes_[end].first.edges.insert(last_edge_id_);
        
        return last_edge_id_;
    }

    void remove_edge(EdgeId id) {
        auto edge_it = edges_.find(id);

        if (edge_it == std::end(edges_)) {
            throw std::out_of_range{fmt::format("Edge {} not found", id)};
        }

        nodes_[edge_it->second.first.begin].first.edges.erase(id);
        nodes_[edge_it->second.first.end].first.edges.erase(id);
        edges_.erase(edge_it);
        
        if (last_edge_id_ == id) {
            --last_edge_id_;
        }
    }

  private:
    template <std::integral Id>
    void update_last_id(Id& last_id, Id hint) {
        if (last_id >= std::numeric_limits<Id>::max()) {
            throw std::length_error{"Max id reached"};
        }

        last_id = last_id < hint ? hint : (last_id + 1);
    }

    std::map<NodeId, std::pair<Node, NodeData>> nodes_;
    std::map<EdgeId, std::pair<Edge, EdgeData>> edges_;

    NodeId last_node_id_{};
    EdgeId last_edge_id_{};
};

struct DefaultEdgeData final {};
struct DefaultNodeData final {
    Point coord;
    NodeType type;
};

using DefaultGraph = Graph<DefaultNodeData, DefaultEdgeData>;

DefaultGraph json_to_graph(const JsonGraph& json);
JsonGraph graph_to_json(const DefaultGraph& graph);
}  // namespace steiner
