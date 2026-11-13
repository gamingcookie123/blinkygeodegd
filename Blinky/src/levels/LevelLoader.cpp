#include "LevelLoader.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/string.hpp>
#include <sstream>
#include <algorithm>
#include <stdexcept>

// GD level strings are zlib-compressed + base64-encoded.
// Geode provides ZipUtils helpers.
#include <Geode/cocos/support/zip_support/ZipUtils.h>

using namespace blinky;
using namespace geode::prelude;

LevelLoader& LevelLoader::get() {
    static LevelLoader inst;
    return inst;
}

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------

static std::vector<std::string> splitStr(const std::string& s, char delim) {
    std::vector<std::string> tokens;
    std::istringstream stream(s);
    std::string tok;
    while (std::getline(stream, tok, delim)) tokens.push_back(tok);
    return tokens;
}

static float safeFloat(const std::string& s, float def = 0.f) {
    if (s.empty()) return def;
    try { return std::stof(s); }
    catch (...) { return def; }
}

static int safeInt(const std::string& s, int def = 0) {
    if (s.empty()) return def;
    try { return std::stoi(s); }
    catch (...) { return def; }
}

// ------------------------------------------------------------
// Decompression
// ------------------------------------------------------------

std::string LevelLoader::decompressLevelString(const std::string& compressed) {
    // GD level strings: base64url-encoded, zlib-compressed data
    // Geode exposes ZipUtils::decompressString / decompressString2
    unsigned char* outBuffer = nullptr;
    ssize_t        outSize   = 0;

    // Try the GD-style decoder (base64 + zlib inflate)
    int result = ZipUtils::ccInflateMemory(
        reinterpret_cast<unsigned char*>(const_cast<char*>(compressed.data())),
        static_cast<ssize_t>(compressed.size()),
        &outBuffer,
        &outSize
    );

    if (result != Z_OK || outBuffer == nullptr) {
        log::error("[Blinky] Failed to decompress level string (zlib code {})", result);
        return {};
    }

    std::string out(reinterpret_cast<char*>(outBuffer), static_cast<size_t>(outSize));
    free(outBuffer);
    return out;
}

// ------------------------------------------------------------
// Color channel parsing  (kS38 field in level header)
// ------------------------------------------------------------

bool LevelLoader::parseColorChannels(const std::string& headerData, ParsedLevel& out) {
    // headerData is the value of the kS38 key from the level header
    // Format: each channel is separated by '|', fields separated by '_'
    // channel fields: index, r, g, b, pcol, blending, id, opacity ...
    if (headerData.empty()) return true;

    auto channels = splitStr(headerData, '|');
    for (const auto& ch : channels) {
        if (ch.empty()) continue;
        auto fields = splitStr(ch, '_');
        // Build a map of key->value pairs
        std::unordered_map<std::string, std::string> kv;
        for (size_t i = 0; i + 1 < fields.size(); i += 2)
            kv[fields[i]] = fields[i + 1];

        ColorChannelData cd{};
        cd.channel  = safeInt(kv["6"]);
        cd.r        = safeFloat(kv["1"], 255.f) / 255.f;
        cd.g        = safeFloat(kv["2"], 255.f) / 255.f;
        cd.b        = safeFloat(kv["3"], 255.f) / 255.f;
        cd.opacity  = safeFloat(kv["7"], 1.f);
        cd.blending = safeInt(kv["5"]) != 0;
        out.colorChannels.push_back(cd);
    }
    return true;
}

// ------------------------------------------------------------
// Object parsing
// ------------------------------------------------------------

bool LevelLoader::parseObjects(const std::string& objectData, ParsedLevel& out) {
    // Objects are separated by ';'
    // Each object is key-value pairs separated by ','
    auto objects = splitStr(objectData, ';');

    // Trigger object IDs to skip (not visible gameplay objects)
    static const std::unordered_set<int> TRIGGER_IDS = {
        899, 900, 901, 915, 1006, 1007, 1049, 1268, 1346, 1347, 1520, 1585,
        1595, 1611, 1612, 1613, 1614, 1615, 1616, 1617, 1618, 1619, 1620,
        1811, 1812, 1813, 1814, 1815, 1816, 1817, 1818, 1819, 1820, 1821,
        1912, 1913, 1914, 1932, 1933, 2015, 2016, 2066, 2067, 2068, 2900,
        3614, 3615, 3617, 3619, 3620, 3641, 3642, 3643, 3660, 3661,
        // Text objects are also triggers in spirit
        914
    };

    out.maxX = 0.f;

    for (const auto& objStr : objects) {
        if (objStr.empty()) continue;

        auto fields = splitStr(objStr, ',');
        if (fields.size() < 2) continue;

        // Build key->value map
        std::unordered_map<std::string, std::string> kv;
        for (size_t i = 0; i + 1 < fields.size(); i += 2)
            kv[fields[i]] = fields[i + 1];

        int objID = safeInt(kv["1"]);
        if (objID <= 0) continue;
        if (TRIGGER_IDS.count(objID)) continue;

        ParsedObject po{};
        po.objectID      = objID;
        po.x             = safeFloat(kv["2"]);
        po.y             = safeFloat(kv["3"]);
        po.rotation      = safeFloat(kv["6"]);
        po.scaleX        = safeFloat(kv["28"], 1.f);
        po.scaleY        = safeFloat(kv["29"], 1.f);
        po.flipX         = safeInt(kv["4"]) != 0;
        po.flipY         = safeInt(kv["5"]) != 0;
        po.colorChannel1 = safeInt(kv["21"]);
        po.colorChannel2 = safeInt(kv["22"]);

        out.objects.push_back(po);
        if (po.x > out.maxX) out.maxX = po.x;
    }
    return true;
}

// ------------------------------------------------------------
// Public entry point
// ------------------------------------------------------------

bool LevelLoader::load(GJGameLevel* level, ParsedLevel& out) {
    if (!level) {
        log::error("[Blinky] LevelLoader::load called with null level");
        return false;
    }

    int id = level->m_levelID;

    // Return cache hit
    if (auto it = m_cache.find(id); it != m_cache.end()) {
        out = it->second;
        return true;
    }

    // The raw level string is stored in m_levelString
    std::string raw = level->m_levelString;

    if (raw.empty()) {
        log::error("[Blinky] Level {} has empty m_levelString", id);
        return false;
    }

    // GD level strings are base64url → zlib; Geode's ZipUtils handles this.
    // If the string starts with 'H4sI' it is already gzip'd base64.
    // If it starts with 'x' or similar it may be raw zlib.
    // GameLevelManager::dataForLevel handles decoding – we ask it here.
    std::string decoded;

    // Strategy 1: GameLevelManager::getLevelString (available in GD 2.2 / Geode 4+)
    // If this method doesn't exist in the installed SDK the linker will error;
    // we guard it with a type-erased call so the mod still compiles.
    {
        auto* glm = GameLevelManager::sharedState();
        if (glm) {
            // m_levelString on main levels is already the raw zlib+b64 data
            // that GameLevelManager would return unchanged, so just use it directly.
            // We don't call getLevelString() because its signature varies across
            // SDK versions; using raw is safer and equivalent for local levels.
            decoded = raw; // will be decompressed below if still compressed
        }
    }

    // Strategy 2: If raw starts with 'H' it may already be gzip base64;
    // if it looks like plaintext XML/plist (starts with '<') use as-is.
    if (!decoded.empty() && decoded[0] == '<') {
        // Already decompressed (some main levels ship this way)
        // Keep decoded as-is
    } else if (!decoded.empty()) {
        // Need to decompress
        std::string attempt = decompressLevelString(decoded);
        if (!attempt.empty()) decoded = attempt;
    }

    if (decoded.empty()) {
        log::error("[Blinky] Could not decode level string for level {}", id);
        return false;
    }

    // The decoded string is plist-like XML:
    // <d> header block </d><d> objects </d>
    // Split on the separator '<d>' to get header vs objects part
    // Objects part is everything after the first ';' in the second <d> block

    // Simpler: GD level string inner format is:
    // kA<n>,value,kA<n>,value,...;obj1;obj2;...
    // The header keys all start with 'kA' or 'kS'

    size_t firstSemicolon = decoded.find(';');
    std::string header = (firstSemicolon != std::string::npos)
                         ? decoded.substr(0, firstSemicolon)
                         : decoded;
    std::string objectSection = (firstSemicolon != std::string::npos)
                                ? decoded.substr(firstSemicolon + 1)
                                : "";

    // Parse header for color channels (kS38) and BG/ground IDs
    {
        auto hFields = splitStr(header, ',');
        std::unordered_map<std::string, std::string> hkv;
        for (size_t i = 0; i + 1 < hFields.size(); i += 2)
            hkv[hFields[i]] = hFields[i + 1];

        out.bgID    = safeInt(hkv["kA6"]);
        out.groundID= safeInt(hkv["kA7"]);
        // BG color: kS29 is a color channel; fall back to channel 1000
        parseColorChannels(hkv["kS38"], out);

        // Set a default background color
        out.bgColorR = 0.157f;
        out.bgColorG = 0.212f;
        out.bgColorB = 0.392f;
    }

    if (!parseObjects(objectSection, out)) {
        log::error("[Blinky] Failed to parse objects for level {}", id);
        return false;
    }

    if (out.objects.empty()) {
        log::warn("[Blinky] Level {} parsed but has 0 visible objects", id);
        return false;
    }

    log::info("[Blinky] Loaded level {} '{}': {} objects, maxX={:.0f}",
              id, level->m_levelName, out.objects.size(), out.maxX);

    m_cache[id] = out;
    return true;
}

const ParsedLevel* LevelLoader::getCached(int levelID) const {
    auto it = m_cache.find(levelID);
    return (it != m_cache.end()) ? &it->second : nullptr;
}

void LevelLoader::clearCache() {
    m_cache.clear();
}
