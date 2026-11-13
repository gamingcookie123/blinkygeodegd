#pragma once
#include <Geode/Geode.hpp>
#include "../levels/LevelData.hpp"

namespace blinky {

// Displayed for exactly one "flash" duration, then transitions to LevelSelectionLayer.
class FlashLayer : public cocos2d::CCLayer {
public:
    // fragment: the region that was selected for this round
    static FlashLayer* create(const LevelFragment& fragment);

    bool init(const LevelFragment& fragment);

private:
    void onFlashHidden();

    LevelFragment m_fragment;
};

} // namespace blinky
