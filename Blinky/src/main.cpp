#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "ui/MainMenuButton.hpp"
#include "ui/BlinkyMenu.hpp"

using namespace geode::prelude;
using namespace cocos2d;

// ============================================================
// MenuLayer hook – inject the BLINK MODE button
// ============================================================

class $modify(BlinkyMenuLayerHook, MenuLayer) {

    // The button callback must be a member of the hooked class
    void onBlinkyButton(CCObject*) {
        auto* menu = blinky::BlinkyMenu::create();
        if (!menu) return;
        CCDirector::sharedDirector()->replaceScene(
            CCTransitionFade::create(0.3f, menu));
    }

    bool init() {
        if (!MenuLayer::init()) return false;

        // Find the row that contains the icon / editor buttons
        // In GD 2.2 these are in a CCMenu near the bottom of the screen
        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // Create the button
        auto* btn = blinky::createBlinkyMainMenuButton(
            this,
            menu_selector(BlinkyMenuLayerHook::onBlinkyButton));

        if (!btn) {
            log::error("[Blinky] Failed to create main menu button");
            return true; // don't crash GD
        }

        // Look for the bottom menu that holds the icon / editor buttons.
        // In GD 2.2 that menu is a child tagged 0 near the bottom-centre.
        // We try to find it by checking positions.
        CCMenu* targetMenu = nullptr;
        CCObject* child;
        CCARRAY_FOREACH(getChildren(), child) {
            auto* m = dynamic_cast<CCMenu*>(child);
            if (!m) continue;
            // The icon/editor row is roughly at y = 25–45
            if (m->getPositionY() < winSize.height * 0.25f && m->getPositionY() > 0.f) {
                targetMenu = m;
                break;
            }
        }

        if (!targetMenu) {
            // Fallback: create our own floating menu
            targetMenu = CCMenu::create();
            targetMenu->setPosition({ 0, 0 });
            addChild(targetMenu, 2);
        }

        // Position: to the right of the existing buttons
        // The editor and icon buttons are roughly at x=winSize.width/2 ± 50
        // Place Blinky button to the right of them
        btn->setPosition({ winSize.width - 50.f, 35.f });
        targetMenu->addChild(btn);

        log::info("[Blinky] Main menu button injected");
        return true;
    }
};
