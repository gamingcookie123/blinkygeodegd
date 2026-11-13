#pragma once
#include <Geode/Geode.hpp>

namespace blinky {

class EndGameLayer : public cocos2d::CCLayer {
public:
    static EndGameLayer* create();
    bool init();

private:
    void onPlayAgain(cocos2d::CCObject* sender);
    void onBlinkyMenu(cocos2d::CCObject* sender);
    void onMainMenu(cocos2d::CCObject* sender);
};

} // namespace blinky
