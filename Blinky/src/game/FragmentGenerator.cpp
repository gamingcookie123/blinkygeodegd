#include "FragmentGenerator.hpp"
#include "../levels/LevelDatabase.hpp"
#include <Geode/Geode.hpp>
#include <random>

using namespace blinky;
using namespace geode::prelude;

FragmentGenerator& FragmentGenerator::get() {
    static FragmentGenerator inst;
    return inst;
}

int FragmentGenerator::countObjectsInRegion(const ParsedLevel& parsed, const LevelFragment& frag) {
    int count = 0;
    float x0 = frag.originX;
    float x1 = frag.originX + frag.widthUnits;
    float y0 = frag.originY;
    float y1 = frag.originY + frag.heightUnits;

    for (const auto& obj : parsed.objects) {
        if (obj.x >= x0 && obj.x <= x1 &&
            obj.y >= y0 && obj.y <= y1) {
            count++;
        }
    }
    return count;
}

bool FragmentGenerator::generate(LevelFragment& out) {
    auto& db  = LevelDatabase::get();
    auto& ldr = LevelLoader::get();

    int fragW = static_cast<int>(
        Mod::get()->getSettingValue<int64_t>("fragment-width-blocks")) * 30;
    int fragH = static_cast<int>(
        Mod::get()->getSettingValue<int64_t>("fragment-height-blocks")) * 30;

    std::random_device rd;
    std::mt19937 gen(rd());

    auto eligible = db.getEligibleLevels();
    if (eligible.empty()) {
        log::error("[Blinky] No eligible levels in database");
        return false;
    }

    // Shuffle level order for fair selection
    std::shuffle(eligible.begin(), eligible.end(), gen);

    for (const auto& info : eligible) {
        GJGameLevel* gj = db.getGJLevel(info.levelID);
        if (!gj) {
            log::warn("[Blinky] Level {} not available locally, skipping", info.levelID);
            continue;
        }

        ParsedLevel parsed;
        if (!ldr.load(gj, parsed)) {
            log::warn("[Blinky] Failed to load level {}", info.levelID);
            continue;
        }

        if (parsed.objects.empty()) continue;

        // Determine valid X range for the fragment origin
        // Avoid the very start (<15 blocks) and very end (last 10 blocks)
        float minX = 15.f * 30.f;
        float maxX = parsed.maxX - static_cast<float>(fragW) - 10.f * 30.f;

        if (maxX < minX) {
            // Level too short – try full range
            minX = 0.f;
            maxX = std::max(0.f, parsed.maxX - static_cast<float>(fragW));
        }
        if (maxX < minX) continue;

        // Y range: typical GD level occupies y≈-120 to y≈480 (units)
        float yCenter = 135.f; // approx middle of a standard level
        float yVariance = 90.f;

        for (int attempt = 0; attempt < MAX_ATTEMPTS; attempt++) {
            std::uniform_real_distribution<float> distX(minX, maxX);
            std::uniform_real_distribution<float> distY(
                yCenter - yVariance, yCenter + yVariance);

            LevelFragment frag{};
            frag.levelID    = info.levelID;
            frag.originX    = distX(gen);
            frag.originY    = distY(gen);
            frag.widthUnits = static_cast<float>(fragW);
            frag.heightUnits= static_cast<float>(fragH);

            if (countObjectsInRegion(parsed, frag) >= MIN_OBJECTS) {
                out = frag;
                log::info("[Blinky] Fragment chosen: level={} x={:.0f} y={:.0f}",
                          frag.levelID, frag.originX, frag.originY);
                return true;
            }
        }
    }

    log::error("[Blinky] FragmentGenerator: exhausted all levels without valid fragment");
    return false;
}
