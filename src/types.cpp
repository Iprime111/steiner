#include "types.hpp"
#include "log.hpp"

namespace steiner {
void to_json(nlohmann::json& j, const steiner::NodeType& type) {
    switch (type) {
    case steiner::NodeType::kSteinerPoint:
        j = "s";
        break;
    case steiner::NodeType::kTerminal:
        j = "t";
      break;
    }
}

void from_json(const nlohmann::json& j, steiner::NodeType& type) {
    auto str = j.get<std::string>();
    
    if (str == "s") {
        type = steiner::NodeType::kSteinerPoint;
    } else if (str == "t") {
        type = steiner::NodeType::kTerminal;
    } else {
        LOG_CRITICAL("Invalid node type: {}", str);
    }
}

void to_json(nlohmann::json& j, const steiner::Node& node) {
    j = {
        {"x", node.coord.x},  
        {"y", node.coord.y},
        {"name", node.name},
        {"id", node.id},  
        {"type", node.type},  
        {"edges", node.edges},  
    };
}

void from_json(const nlohmann::json& j, steiner::Node& node) {
    j.at("x").get_to(node.coord.x);
    j.at("y").get_to(node.coord.y);
    j.at("name").get_to(node.name);
    j.at("id").get_to(node.id);
    j.at("type").get_to(node.type);
    
    if (j.contains("edges")) {
        j.at("edges").get_to(node.edges);
    } else {
        node.edges = {};
    }
}

void to_json(nlohmann::json& j, const steiner::Edge& edge) {
    j = {
        {"id", edge.id},
        {"vertices", edge.vertices},
    };
}

void from_json(const nlohmann::json& j, steiner::Edge& edge) {
    j.at("id").get_to(edge.id);
    j.at("vertices").get_to(edge.vertices);
}

void to_json(nlohmann::json& j, const steiner::AlgoCtx& ctx) {
    j = {
        {"node", ctx.node},
        {"edge", ctx.edge},
    };
}

void from_json(const nlohmann::json& j, steiner::AlgoCtx& ctx) {
    j.at("node").get_to(ctx.node);
    j.at("edge").get_to(ctx.edge);
}
} // namespace steiner