#pragma once
#include <Geode/Geode.hpp>
#include <vector>
#include <string>

namespace blinky {

// A minimal parsed object for rendering purposes
struct ParsedObject {
    int   objectID;
    float x;        // level units
    float y;        // level units
    float rotation; // degrees
    float scaleX;
    float scaleY;
    bool  flipX;
    bool  flipY;
    // color channel indices (0 = none)
    int   colorChannel1;
    int   colorChannel2;
};

// Color channel data extracted from the level header
struct ColorChannelData {
    int   channel;
    float r, g, b;
    float opacity;
    bool  blending;
};

struct ParsedLevel {
    std::vector<ParsedObject>    objects;
    std::vector<ColorChannelData> colorChannels;
    float bgColorR, bgColorG, bgColorB;
    int   bgID;    // background sprite index
    int   groundID;// ground sprite index
    float maxX;    // rightmost object X
};

class LevelLoader {
public:
    static LevelLoader& get();

    // Parse the raw level string of a GJGameLevel into usable object list.
    // Returns false and logs an error if parsing fails.
    bool load(GJGameLevel* level, ParsedLevel& out);

    // Returns a cached result if the same level was parsed before
    const ParsedLevel* getCached(int levelID) const;
    void clearCache();

private:
    LevelLoader() = default;

    std::string decompressLevelString(const std::string& compressed);
    bool parseObjects(const std::string& objectData, ParsedLevel& out);
    bool parseColorChannels(const std::string& headerData, ParsedLevel& out);

    // key = levelID
    std::unordered_map<int, ParsedLevel> m_cache;
};

} // namespace blinky
