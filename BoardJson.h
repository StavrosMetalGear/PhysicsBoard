#pragma once
#include "Board.h"
#include <nlohmann/json.hpp>

// JSON serialization for Point
inline void to_json(nlohmann::json& j, const Point& p) {
    j = nlohmann::json{{"x", p.x}, {"y", p.y}};
}
inline void from_json(const nlohmann::json& j, Point& p) {
    j.at("x").get_to(p.x);
    j.at("y").get_to(p.y);
}

// JSON serialization for Stroke
inline void to_json(nlohmann::json& j, const Stroke& s) {
    j = nlohmann::json{
        {"tool", static_cast<int>(s.tool)},
        {"points", s.points},
        {"color", s.color},
        {"thickness", s.thickness},
        {"text", s.text}
    };
}
inline void from_json(const nlohmann::json& j, Stroke& s) {
    int toolInt;
    j.at("tool").get_to(toolInt);
    s.tool = static_cast<ToolType>(toolInt);
    j.at("points").get_to(s.points);
    j.at("color").get_to(s.color);
    j.at("thickness").get_to(s.thickness);
    j.at("text").get_to(s.text);
}

// JSON serialization for Board (strokes only)
inline void to_json(nlohmann::json& j, const Board& b) {
    j = nlohmann::json{{"strokes", b.getStrokes()}};
}
inline void from_json(const nlohmann::json& j, Board& b) {
    std::vector<Stroke> strokes;
    j.at("strokes").get_to(strokes);
    for (const auto& s : strokes) b.addStroke(s);
}
