#pragma once

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <nlohmann/json.hpp>

namespace steiner {
using Distance = long;
using NodeId = long;
using EdgeId = long;

enum class NodeType {
    kSteinerPoint,
    kTerminal,
};

struct Point final {
    Point() = default;
    Point(Distance x, Distance y) : x(x), y(y) {}

    template <typename Container>
        requires(!std::is_same_v<std::decay_t<Container>, Point>) &&
                    (std::tuple_size_v<std::decay_t<Container>> == 2)
    Point(Container&& coord)
        : x(std::get<0>(std::forward<Container>(coord))), y(std::get<1>(std::forward<Container>(coord))) {}

    bool operator==(const Point& other) const = default;

    Distance x{};
    Distance y{};
};

NLOHMANN_JSON_SERIALIZE_ENUM(NodeType, {{NodeType::kSteinerPoint, "s"}, {NodeType::kTerminal, "t"}})

struct JsonNode final {
    NodeId id;
    Point coord;
    NodeType type;
    std::vector<EdgeId> edges;
};

struct JsonEdge final {
    EdgeId id;
    std::vector<NodeId> vertices;
};

struct JsonGraph {
    std::vector<JsonNode> node;
    std::vector<JsonEdge> edge;
};

void to_json(nlohmann::json& j, const JsonNode& n);
void from_json(const nlohmann::json& j, JsonNode& n);
void to_json(nlohmann::json& j, const JsonEdge& e);
void from_json(const nlohmann::json& j, JsonEdge& e);
void to_json(nlohmann::json& j, const JsonGraph& g);
void from_json(const nlohmann::json& j, JsonGraph& g);
}  // namespace steiner

template <>
struct fmt::formatter<steiner::Point> : formatter<string_view> {
    auto format(steiner::Point p, format_context& ctx) const {
        return fmt::format_to(ctx.out(), "Point(x={}, y={})", p.x, p.y);
    }
};

template <>
struct fmt::formatter<steiner::NodeType> : formatter<string_view> {
    auto format(steiner::NodeType t, format_context& ctx) const {
        auto name = (t == steiner::NodeType::kSteinerPoint) ? "s" : "t";
        return formatter<string_view>::format(name, ctx);
    }
};

template <>
struct fmt::formatter<steiner::JsonNode> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    auto format(const steiner::JsonNode& n, format_context& ctx) const {
        return fmt::format_to(ctx.out(), "Node(id={}, coord={}, type={}, edges=[{}])", n.id, n.coord, n.type,
                              fmt::join(n.edges, ", "));
    }
};

template <>
struct fmt::formatter<steiner::JsonEdge> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    auto format(const steiner::JsonEdge& e, format_context& ctx) const {
        return fmt::format_to(ctx.out(), "Edge(id={}, vertices={})", e.id, e.vertices);
    }
};

template <>
struct fmt::formatter<steiner::JsonGraph> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    auto format(const steiner::JsonGraph& g, format_context& ctx) const {
        auto out = ctx.out();

        if (g.node.empty() && g.edge.empty()) {
            return fmt::format_to(out, "Graph is empty.");
        }

        if (!g.node.empty()) {
            fmt::format_to(out, "======== Nodes ========\n{}\n", fmt::join(g.node, "\n"));
        }

        if (!g.edge.empty()) {
            fmt::format_to(out, "======== Edges ========\n{}\n", fmt::join(g.edge, "\n"));
        }

        return out;
    }
};

namespace std {
template <>
struct hash<steiner::Point> {
    size_t operator()(const steiner::Point& p) const noexcept {
        size_t h1 = hash<int>{}(p.x);
        size_t h2 = hash<int>{}(p.y);

        // Formula is taken from the Boost library (boost::hash_combine)
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};
}  // namespace std
