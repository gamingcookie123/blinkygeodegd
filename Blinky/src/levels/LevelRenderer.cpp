#include "LevelRenderer.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/GameManager.hpp>
#include <algorithm>

using namespace blinky;
using namespace geode::prelude;
using namespace cocos2d;

LevelRenderer& LevelRenderer::get() {
    static LevelRenderer inst;
    return inst;
}

// ------------------------------------------------------------
// Object ID → sprite frame name mapping
//
// GD stores objects in GJ_GameSheet.plist / GJ_GameSheet2.plist etc.
// The mapping below covers the most common block IDs.
// Unknown IDs fall back to "square_01_001.png" (a plain white square).
// ------------------------------------------------------------

std::string LevelRenderer::frameNameForObjectID(int id) {
    if (auto it = m_frameCache.find(id); it != m_frameCache.end())
        return it->second;

    static const std::unordered_map<int, std::string> TABLE = {
        // --- Main blocks ---
        {  1, "square_01_001.png" },
        {  2, "square_02_001.png" },
        {  3, "square_03_001.png" },
        {  4, "square_04_001.png" },
        {  5, "square_05_001.png" },
        {  6, "square_06_001.png" },
        {  7, "spike_01_001.png"  },
        {  8, "spike_02_001.png"  },
        {  9, "spike_03_001.png"  },
        { 10, "spike_04_001.png"  },
        { 11, "spike_05_001.png"  },
        { 12, "spike_06_001.png"  },
        { 13, "square_07_001.png" },
        { 14, "square_08_001.png" },
        { 15, "square_09_001.png" },
        { 17, "square_11_001.png" },
        { 18, "square_12_001.png" },
        { 19, "square_13_001.png" },
        { 20, "square_14_001.png" },
        // Ship portals / game portals
        { 11, "portal_01_001.png" },
        // Ring / orbs
        {  36, "ring_01_001.png"  },
        {  84, "ring_02_001.png"  },
        { 141, "ring_03_001.png"  },
        { 1022, "ring_04_001.png" },
        { 1333, "ring_05_001.png" },
        // Pads / jump pads
        {  67, "pad_01_001.png"   },
        { 140, "pad_02_001.png"   },
        { 1332, "pad_03_001.png"  },
        // Gravity portal
        { 10, "portal_02_001.png" },
        // Coin
        {   142, "coin_01_001.png" },
        { 1275,  "coin_01_001.png" },
        // --- Decorative / environment ---
        { 211, "half_01_001.png"  },
        { 212, "half_02_001.png"  },
        { 213, "half_03_001.png"  },
    };

    std::string frame = "square_01_001.png"; // safe fallback
    if (auto it = TABLE.find(id); it != TABLE.end())
        frame = it->second;

    m_frameCache[id] = frame;
    return frame;
}

CCColor3B LevelRenderer::resolveColor(const ParsedLevel& level, int channel) {
    if (channel == 0) return { 255, 255, 255 };
    for (const auto& cd : level.colorChannels) {
        if (cd.channel == channel) {
            return {
                static_cast<GLubyte>(cd.r * 255),
                static_cast<GLubyte>(cd.g * 255),
                static_cast<GLubyte>(cd.b * 255)
            };
        }
    }
    return { 255, 255, 255 };
}

// ------------------------------------------------------------
// renderRegion
// ------------------------------------------------------------

CCNode* LevelRenderer::renderRegion(
    const ParsedLevel& level,
    const RenderRegion& region,
    float pixelsPerUnit)
{
    // Container node – sized to the requested region in output pixels
    auto* container = CCNode::create();
    float outW = region.width  * pixelsPerUnit;
    float outH = region.height * pixelsPerUnit;
    container->setContentSize({ outW, outH });

    // Background color layer
    auto* bg = CCLayerColor::create(
        { static_cast<GLubyte>(level.bgColorR * 255),
          static_cast<GLubyte>(level.bgColorG * 255),
          static_cast<GLubyte>(level.bgColorB * 255),
          255 },
        outW, outH
    );
    container->addChild(bg, -1);

    // Collect objects inside the region (with 1 block margin so edges look clean)
    float margin = 30.f; // 1 block
    float xMin = region.originX - margin;
    float xMax = region.originX + region.width  + margin;
    float yMin = region.originY - margin;
    float yMax = region.originY + region.height + margin;

    auto* sheet = CCSpriteFrameCache::sharedSpriteFrameCache();
    int added = 0;

    for (const auto& obj : level.objects) {
        if (obj.x < xMin || obj.x > xMax) continue;
        if (obj.y < yMin || obj.y > yMax) continue;

        // Convert level coords → node-local coords
        float localX = (obj.x - region.originX) * pixelsPerUnit;
        float localY = (obj.y - region.originY) * pixelsPerUnit;

        // Try to create sprite from cached frame; fall back to colored square
        std::string frameName = frameNameForObjectID(obj.objectID);
        CCSprite* sprite = nullptr;

        auto* frame = sheet->spriteFrameByName(frameName.c_str());
        if (frame) {
            sprite = CCSprite::createWithSpriteFrame(frame);
        } else {
            sprite = CCSprite::create();
            if (!sprite) continue;
            // Draw a coloured rectangle as fallback using CCLayerColor
            auto* rect = CCLayerColor::create({ 100, 150, 200, 200 },
                30.f * pixelsPerUnit, 30.f * pixelsPerUnit);
            rect->setPosition({ localX, localY });
            container->addChild(rect, 1);
            added++;
            continue;
        }

        if (!sprite) continue;

        // Scale to match pixelsPerUnit (GD blocks are 30 units wide/tall)
        float baseSize = 30.f * pixelsPerUnit;
        auto naturalSize = sprite->getContentSize();
        if (naturalSize.width > 0 && naturalSize.height > 0) {
            sprite->setScale(baseSize / naturalSize.width * obj.scaleX);
            sprite->setScaleY(baseSize / naturalSize.height * obj.scaleY);
        }

        sprite->setFlipX(obj.flipX);
        sprite->setFlipY(obj.flipY);
        sprite->setRotation(obj.rotation);
        sprite->setPosition({ localX, localY });

        // Apply color
        if (obj.colorChannel1 != 0) {
            sprite->setColor(resolveColor(level, obj.colorChannel1));
        }

        container->addChild(sprite, 1);
        added++;
    }

    log::debug("[Blinky] renderRegion: added {} sprites", added);
    return container;
}

// ------------------------------------------------------------
// renderFullLevel
// ------------------------------------------------------------

CCNode* LevelRenderer::renderFullLevel(const ParsedLevel& level, float pixelsPerUnit) {
    RenderRegion full{};
    full.originX = 0.f;
    full.originY = -150.f;             // GD ground is around y=0; go below
    full.width   = level.maxX + 300.f; // extra margin at right
    full.height  = 600.f;              // covers most level heights

    return renderRegion(level, full, pixelsPerUnit);
}
