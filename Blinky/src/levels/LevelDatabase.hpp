#pragma once
#include "LevelData.hpp"
#include <Geode/Geode.hpp>
#include <optional>

namespace blinky {

class LevelDatabase {
public:
    static LevelDatabase& get();

    // Returns a filtered list based on the current settings toggles
    std::vector<LevelInfo> getEligibleLevels() const;

    // Try to fetch the actual GJGameLevel* from the game.
    // Returns nullptr if the level is not available locally.
    GJGameLevel* getGJLevel(int levelID) const;

    // Pick a random eligible level
    std::optional<LevelInfo> pickRandom() const;

private:
    LevelDatabase() = default;
};

} // namespace blinky
