#pragma once
#include "LevelLoader.hpp"
#include <Geode/Geode.hpp>

namespace blinky {

// Options for what region to render
struct RenderRegion {
    float originX;      // level units (left edge)
    float originY;      // level units (bottom edge)
    float width;        // level units
    float height;       // level units
};

class LevelRenderer {
public:
    static LevelRenderer& get();

    // Render a rectangular region of a parsed level into a new CCNode.
    // The returned node's size equals (width * scale, height * scale) in points.
    // scale: how many points per level unit.  Default: 1.0 / 30.0 * tileSize
    // tileSize: pixel size per block in the output (default 30px per block = 1pt per unit)
    // The caller owns the returned node.
    cocos2d::CCNode* renderRegion(
        const ParsedLevel& level,
        const RenderRegion& region,
        float pixelsPerUnit = 1.0f   // output pixels per level coordinate unit
    );

    // Render the full level into a scrollable CCNode (no clip).
    // pixelsPerUnit controls the zoom level.
    cocos2d::CCNode* renderFullLevel(
        const ParsedLevel& level,
        float pixelsPerUnit = 0.5f
    );

private:
    LevelRenderer() = default;

    // Look up the frame name for a GD object ID.
    // Returns an empty string if the frame is unknown.
    std::string frameNameForObjectID(int objectID);

    // Resolve a color from the channel table; returns white if not found
    cocos2d::ccColor3B resolveColor(const ParsedLevel& level, int channel);

    // Frame name cache
    std::unordered_map<int, std::string> m_frameCache;
};

} // namespace blinky
