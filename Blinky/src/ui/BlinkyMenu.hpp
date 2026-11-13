#pragma once
#include <Geode/Geode.hpp>
#include "../game/GameState.hpp"

namespace blinky {

class BlinkyMenu : public cocos2d::CCLayer {
public:
    static BlinkyMenu* create();
    bool init();

private:
    void onPlayMode(cocos2d::CCObject* sender);
    void onSettings(cocos2d::CCObject* sender);
    void onBack(cocos2d::CCObject* sender);

    GameMode m_selectedMode = GameMode::Rounds5;
    cocos2d::CCArray* m_modeButtons = nullptr;
    void updateModeHighlight();
};

} // namespace blinky
