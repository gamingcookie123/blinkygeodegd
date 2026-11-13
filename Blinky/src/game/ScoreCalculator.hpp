#pragma once
#include "../game/GameState.hpp"

namespace blinky {

class ScoreCalculator {
public:
    // Euclidean distance in level units
    static float distanceUnits(float x1, float y1, float x2, float y2);

    // Penalty in blocks (round(distance / 30))
    static int penaltyBlocks(float distUnits);

    // Optionally add a flat penalty if the wrong level was chosen
    static int wrongLevelPenalty();

    // Build a RoundResult and push it to gState
    static RoundResult buildResult(
        const LevelFragment& fragment,
        int guessedLevelID,
        float guessedX,
        float guessedY
    );
};

} // namespace blinky
