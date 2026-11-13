#include "MainMenuButton.hpp"
#include <Geode/Geode.hpp>

using namespace blinky;
using namespace geode::prelude;
using namespace cocos2d;

CCMenuItemSpriteExtra* blinky::createBlinkyMainMenuButton(
    CCObject* target, SEL_MenuHandler selector)
{
    // Attempt to use the GD play button sprite for visual consistency
    // "GJ_playBtn2_001.png" is the smaller play button used in the editor row
    CCSprite* icon = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
    if (!icon) {
        // Fallback: plain text button
        icon = CCSprite::create();
        icon->setContentSize({ 100.f, 36.f });
        auto* bg = CCScale9Sprite::create("GJ_button_02.png");
        bg->setContentSize({ 100.f, 36.f });
        bg->setPosition({ 50.f, 18.f });
        icon->addChild(bg);
    }

    // Scale down to roughly half size relative to the main play button
    icon->setScale(0.75f);

    // "BLINK MODE" label below the icon
    auto* lbl = CCLabelBMFont::create("BLINK MODE", "bigFont.fnt");
    lbl->setScale(0.30f);
    lbl->setColor({ 255, 220, 50 });

    // Container node
    auto* container = CCSprite::create();
    auto iconSize = icon->getContentSize() * icon->getScale();
    container->setContentSize({
        std::max(iconSize.width, 100.f),
        iconSize.height + 16.f
    });

    icon->setPosition({ container->getContentSize().width * 0.5f,
                        iconSize.height * 0.5f + 14.f });
    lbl->setPosition({ container->getContentSize().width * 0.5f, 8.f });
    container->addChild(icon);
    container->addChild(lbl);

    return CCMenuItemSpriteExtra::create(container, nullptr, target, selector);
}
