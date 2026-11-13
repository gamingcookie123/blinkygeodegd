#pragma once
#include <Geode/Geode.hpp>
#include "../game/GameState.hpp"

namespace blinky {

class ResultsLayer : public cocos2d::CCLayer {
public:
    static ResultsLayer* create(const RoundResult& result);
    bool init(const RoundResult& result);

private:
    void onNextRound(cocos2d::CCObject* sender);
    void onEndGame(cocos2d::CCObject* sender);

    RoundResult m_result;
};

} // namespace blinky
