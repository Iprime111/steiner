#pragma once

#include <tuple>
#include <utility>
#include <vector>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>

namespace steiner {
using Distance = long;

enum class NodeType {
    kSteinerPoint,
    kTerminal,
};

using NodeId = long;
using EdgeId = long;

struct Point final {
    Point() = default;
    Point(Distance x, Distance y) : x(x), y(y) {}
    
    template <typename Container>
    requires (!std::is_same_v<std::decay_t<Container>, Point>) && 
             (std::tuple_size_v<std::decay_t<Container>> == 2)
    Point(Container&& coord) : 
        x(std::get<0>(std::forward<Container>(coord))), y(std::get<1>(std::forward<Container>(coord))) {}
        
    bool operator==(const Point& other) const = default;
    
    Distance x{};
    Distance y{};
};

struct Node final {
    Point coord;
    std::string name;
    NodeId id{};
    NodeType type;
    std::vector<EdgeId> edges;
};

struct Edge final {
    EdgeId id{};
    std::vector<NodeId> vertices;
};

struct AlgoCtx final {
    std::vector<Node> node;
    std::vector<Edge> edge;
};

void to_json(nlohmann::json &j, const steiner::NodeType &type);
void from_json(const nlohmann::json &j, steiner::NodeType &type);

void to_json(nlohmann::json &j, const steiner::Node &node);
void from_json(const nlohmann::json &j, steiner::Node &node);

void to_json(nlohmann::json &j, const steiner::Edge &node);
void from_json(const nlohmann::json &j, steiner::Edge &node);

void to_json(nlohmann::json &j, const steiner::AlgoCtx &ctx);
void from_json(const nlohmann::json &j, steiner::AlgoCtx &ctx);
} // namespace steiner

template <>
struct fmt::formatter<steiner::NodeType> : formatter<string_view> {
    auto format(steiner::NodeType t, format_context& ctx) const {
        auto name = (t == steiner::NodeType::kSteinerPoint) ? "s" : "t";
        return formatter<string_view>::format(name, ctx);
    }
};

template <>
struct fmt::formatter<steiner::Node> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    auto format(const steiner::Node& n, format_context& ctx) const {
        return fmt::format_to(ctx.out(), "Node(name={}, id={}, x={}, y={}, type={}, edges=[{}])",
            n.name, n.id, n.coord.x, n.coord.y, n.type, fmt::join(n.edges, ", "));
    }
};

template <>
struct fmt::formatter<steiner::Edge> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    auto format(const steiner::Edge& e, format_context& ctx) const {
        return fmt::format_to(ctx.out(), "Edge(id={}, vertices={})", 
                              e.id, e.vertices);
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
}