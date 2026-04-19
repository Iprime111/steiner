#include "types.hpp"

#include <nlohmann/json.hpp>

namespace steiner {
using json = nlohmann::json;

void to_json(json& j, const JsonNode& n) {
    j = json{{"id", n.id},     {"name", n.name}, {"x", n.coord.x},
             {"y", n.coord.y}, {"type", n.type}, {"edges", n.edges}};
}

void from_json(const json& j, JsonNode& n) {
    j.at("id").get_to(n.id);
    j.at("x").get_to(n.coord.x);
    j.at("y").get_to(n.coord.y);
    j.at("type").get_to(n.type);

    if (j.contains("edges")) {
        j.at("edges").get_to(n.edges);
    } else {
        n.edges.clear();
    }

    if (j.contains("name")) {
        j.at("name").get_to(n.name);
    } else {
        n.name.clear();
    }
}

void to_json(json& j, const JsonEdge& e) {
    j = json{{"id", e.id}, {"vertices", e.vertices}};
}

void from_json(const json& j, JsonEdge& e) {
    j.at("id").get_to(e.id);
    if (j.contains("vertices")) {
        j.at("vertices").get_to(e.vertices);
    } else {
        e.vertices.clear();
    }
}

void to_json(json& j, const JsonGraph& g) {
    j = json{{"node", g.node}};

    if (!g.edge.empty()) {
        j["edge"] = g.edge;
    } else {
        j["edge"] = json::array();
    }
}

void from_json(const json& j, JsonGraph& g) {
    if (j.contains("node")) {
        j.at("node").get_to(g.node);
    }
    if (j.contains("edge")) {
        j.at("edge").get_to(g.edge);
    }
}
}  // namespace steiner
