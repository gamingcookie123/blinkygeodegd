#pragma once
#include "../levels/LevelData.hpp"
#include <vector>
#include <string>

namespace blinky {

enum class GameMode { Rounds5, Rounds10, Endless };

struct RoundResult {
    int   targetLevelID;
    int   guessedLevelID;
    float targetX;    // level units
    float targetY;    // level units
    float guessedX;   // level units
    float guessedY;   // level units
    float distanceUnits;
    int   penaltyBlocks;  // = round(distanceUnits / 30)
    bool  correctLevel;
};

struct GameState {
    GameMode mode = GameMode::Rounds5;

    // Current fragment
    LevelFragment currentFragment{};
    int           currentRoundIndex = 0;

    // Results per round
    std::vector<RoundResult> results;

    // Totals
    int totalPenalty = 0;

    // Derived helpers
    int maxRounds() const {
        switch (mode) {
            case GameMode::Rounds5:  return 5;
            case GameMode::Rounds10: return 10;
            case GameMode::Endless:  return 999999;
        }
        return 5;
    }

    bool isOver() const {
        return mode != GameMode::Endless
               && currentRoundIndex >= maxRounds();
    }

    void reset() {
        currentRoundIndex = 0;
        results.clear();
        totalPenalty = 0;
        currentFragment = {};
    }
};

// Singleton
inline GameState& gState() {
    static GameState s;
    return s;
}

} // namespace blinky
