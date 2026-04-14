#include "graph.hpp"
#include "log.hpp"
#include "types.hpp"

namespace steiner {
DefaultGraph json_to_graph(const JsonGraph& json) {
    using NodeDataType = DefaultGraph::NodeDataType;

    std::unordered_map<NodeId, NodeDataType> init_nodes;
    for (const auto& j_node : json.node) {
        init_nodes.emplace(j_node.id, NodeDataType{j_node.coord, j_node.type});
    }

    DefaultGraph graph{std::move(init_nodes)};

    if (!json.edge.empty()) {
        LOG_CRITICAL("Non-empty edges list in input graph!");
    }

    return graph;
}

JsonGraph graph_to_json(const DefaultGraph& graph) {
    JsonGraph json;

    // Sorting nodes/edges for deterministic json output
    auto node_ids = graph.node_ids() | std::ranges::to<std::vector>();
    std::ranges::sort(node_ids);

    for (auto node_id : node_ids) {
        JsonNode jn;
        jn.id = node_id;

        const auto& data = graph.node_data(node_id);
        jn.coord = data.coord;
        jn.type = data.type;

        const auto& edges = graph.node_edges(node_id);
        jn.edges.assign(edges.begin(), edges.end());
        std::ranges::sort(jn.edges);

        json.node.push_back(std::move(jn));
    }
    auto edge_ids = graph.edge_ids() | std::ranges::to<std::vector>();
    std::ranges::sort(edge_ids);

    for (auto edge_id : edge_ids) {
        JsonEdge je;
        je.id = edge_id;

        auto [edge_begin, edge_end] = graph.edge_nodes(edge_id);

        je.vertices.push_back(edge_begin);
        je.vertices.push_back(edge_end);
        std::ranges::sort(je.vertices);

        json.edge.push_back(std::move(je));
    }

    return json;
}
}  // namespace steiner
