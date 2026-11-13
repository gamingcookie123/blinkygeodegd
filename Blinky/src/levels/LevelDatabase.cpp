#include "LevelDatabase.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <random>

using namespace blinky;
using namespace geode::prelude;

LevelDatabase& LevelDatabase::get() {
    static LevelDatabase instance;
    return instance;
}

std::vector<LevelInfo> LevelDatabase::getEligibleLevels() const {
    bool wantMain    = Mod::get()->getSettingValue<bool>("include-main-levels");
    bool wantTower   = Mod::get()->getSettingValue<bool>("include-tower-levels");

    std::vector<LevelInfo> out;
    for (const auto& info : ALL_OFFICIAL_LEVELS) {
        if (info.category == LevelCategory::Main    && wantMain)    out.push_back(info);
        if (info.category == LevelCategory::Tower   && wantTower)   out.push_back(info);
        if (info.category == LevelCategory::Special && wantMain)    out.push_back(info);
    }
    return out;
}

GJGameLevel* LevelDatabase::getGJLevel(int levelID) const {
    auto* glm = GameLevelManager::sharedState();
    if (!glm) return nullptr;

    // Official levels 1-22 are stored locally by GD
    if (levelID >= 1 && levelID <= 22) {
        auto* lvl = glm->getMainLevel(levelID, false);
        return lvl;
    }

    // The Challenge (1001)
    if (levelID == 1001) {
        return glm->getMainLevel(1001, false);
    }

    // Tower/platformer levels: they use a different access path
    // In GD 2.2 these are accessed via getSecretLevel or similar
    // GameLevelManager has mapPacks and special level storage
    // We fall back to a local cache search
    auto* savedLevels = glm->m_savedLevels;
    if (savedLevels) {
        // m_savedLevels is a CCArray of GJGameLevel*
        for (int i = 0; i < savedLevels->count(); i++) {
            auto* lvl = static_cast<GJGameLevel*>(savedLevels->objectAtIndex(i));
            if (lvl && lvl->m_levelID == levelID) return lvl;
        }
    }

    return nullptr;
}

std::optional<LevelInfo> LevelDatabase::pickRandom() const {
    auto eligible = getEligibleLevels();
    if (eligible.empty()) return std::nullopt;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, eligible.size() - 1);
    return eligible[dist(gen)];
}
