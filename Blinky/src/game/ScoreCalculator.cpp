#include "ScoreCalculator.hpp"
#include <Geode/Geode.hpp>
#include <cmath>

using namespace blinky;
using namespace geode::prelude;

float ScoreCalculator::distanceUnits(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return std::sqrt(dx * dx + dy * dy);
}

int ScoreCalculator::penaltyBlocks(float distUnits) {
    return static_cast<int>(std::round(distUnits / 30.f));
}

int ScoreCalculator::wrongLevelPenalty() {
    bool enabled = Mod::get()->getSettingValue<bool>("wrong-level-penalty");
    return enabled ? 50 : 0;
}

RoundResult ScoreCalculator::buildResult(
    const LevelFragment& fragment,
    int guessedLevelID,
    float guessedX,
    float guessedY)
{
    // Target is the centre of the fragment
    float targetX = fragment.originX + fragment.widthUnits  * 0.5f;
    float targetY = fragment.originY + fragment.heightUnits * 0.5f;

    float dist = distanceUnits(targetX, targetY, guessedX, guessedY);
    int penalty = penaltyBlocks(dist);

    bool correctLevel = (guessedLevelID == fragment.levelID);
    if (!correctLevel) penalty += wrongLevelPenalty();

    RoundResult r{};
    r.targetLevelID   = fragment.levelID;
    r.guessedLevelID  = guessedLevelID;
    r.targetX         = targetX;
    r.targetY         = targetY;
    r.guessedX        = guessedX;
    r.guessedY        = guessedY;
    r.distanceUnits   = dist;
    r.penaltyBlocks   = penalty;
    r.correctLevel    = correctLevel;

    gState().results.push_back(r);
    gState().totalPenalty += penalty;
    gState().currentRoundIndex++;

    return r;
}
