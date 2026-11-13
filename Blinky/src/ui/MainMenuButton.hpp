#pragma once
#include <Geode/Geode.hpp>

namespace blinky {

// The actual button injection is done via a Geode hook in main.cpp
// (MenuLayer::init). This header just declares a helper.

cocos2d::CCMenuItemSpriteExtra* createBlinkyMainMenuButton(
    cocos2d::CCObject* target,
    cocos2d::SEL_MenuHandler selector);

} // namespace blinky
