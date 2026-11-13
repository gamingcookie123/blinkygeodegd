#include "FlashLayer.hpp"
#include "LevelSelectionLayer.hpp"
#include "../game/FlashController.hpp"
#include "../game/GameState.hpp"
#include "../levels/LevelDatabase.hpp"
#include "../levels/LevelLoader.hpp"
#include "../levels/LevelRenderer.hpp"
#include <Geode/Geode.hpp>

using namespace blinky;
using namespace geode::prelude;
using namespace cocos2d;

FlashLayer* FlashLayer::create(const LevelFragment& fragment) {
    auto* obj = new FlashLayer();
    if (obj && obj->init(fragment)) {
        obj->autorelease();
        return obj;
    }
    CC_SAFE_DELETE(obj);
    return nullptr;
}

bool FlashLayer::init(const LevelFragment& fragment) {
    if (!CCLayer::init()) return false;
    m_fragment = fragment;

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    // Dark full-screen background
    auto* bg = CCLayerColor::create({ 10, 10, 20, 255 });
    addChild(bg, 0);

    // Round counter label
    auto* roundLabel = CCLabelBMFont::create(
        ("Round " + std::to_string(gState().currentRoundIndex + 1) +
         " / " + std::to_string(gState().maxRounds() == 999999 ?
             999 : gState().maxRounds())).c_str(),
        "bigFont.fnt");
    roundLabel->setScale(0.5f);
    roundLabel->setPosition({ winSize.width * 0.5f, winSize.height - 20.f });
    addChild(roundLabel, 2);

    // "BLINK!" flash text that disappears with the fragment
    auto* blinkLabel = CCLabelBMFont::create("BLINK!", "bigFont.fnt");
    blinkLabel->setScale(0.9f);
    blinkLabel->setPosition({ winSize.width * 0.5f, winSize.height * 0.5f + 150.f });
    blinkLabel->setColor({ 255, 220, 50 });
    addChild(blinkLabel, 2);

    // Load and render the fragment
    auto& db  = LevelDatabase::get();
    auto& ldr = LevelLoader::get();
    auto& rnd = LevelRenderer::get();

    GJGameLevel* gj = db.getGJLevel(fragment.levelID);
    if (!gj) {
        auto* err = CCLabelBMFont::create("Error: level not available", "bigFont.fnt");
        err->setScale(0.5f);
        err->setPosition({ winSize.width * 0.5f, winSize.height * 0.5f });
        addChild(err, 2);
        // Still transition after delay
        scheduleOnce([this](float) { onFlashHidden(); }, 1.0f, "err_delay");
        return true;
    }

    ParsedLevel parsed;
    if (!ldr.load(gj, parsed)) {
        auto* err = CCLabelBMFont::create("Error: level parse failed", "bigFont.fnt");
        err->setScale(0.5f);
        err->setPosition({ winSize.width * 0.5f, winSize.height * 0.5f });
        addChild(err, 2);
        scheduleOnce([this](float) { onFlashHidden(); }, 1.0f, "err_delay");
        return true;
    }

    // Decide output size: fit fragment into most of the screen
    float maxDisplayW = winSize.width  * 0.9f;
    float maxDisplayH = winSize.height * 0.7f;
    float scaleX = maxDisplayW / fragment.widthUnits;
    float scaleY = maxDisplayH / fragment.heightUnits;
    float scale  = std::min(scaleX, scaleY);

    RenderRegion region{};
    region.originX = fragment.originX;
    region.originY = fragment.originY;
    region.width   = fragment.widthUnits;
    region.height  = fragment.heightUnits;

    CCNode* fragmentNode = rnd.renderRegion(parsed, region, scale);
    if (!fragmentNode) {
        scheduleOnce([this](float) { onFlashHidden(); }, 0.5f, "err_delay");
        return true;
    }

    // Add a thin border frame
    auto fragSize = fragmentNode->getContentSize();
    auto* border = CCDrawNode::create();
    CCPoint corners[4] = {
        { -2, -2 },
        { fragSize.width + 2, -2 },
        { fragSize.width + 2, fragSize.height + 2 },
        { -2, fragSize.height + 2 }
    };
    border->drawPolygon(corners, 4,
        { 1, 1, 1, 0 },   // fill alpha=0
        2.f,
        { 1, 0.86f, 0.2f, 1 }  // golden border
    );
    fragmentNode->addChild(border, 10);

    // Centre fragment on screen
    fragmentNode->setPosition({
        (winSize.width  - fragSize.width)  * 0.5f,
        (winSize.height - fragSize.height) * 0.5f - 20.f
    });
    fragmentNode->setVisible(false); // FlashController will show it
    addChild(fragmentNode, 1);

    // FlashController manages visibility
    auto* fc = FlashController::create(fragmentNode, [this]() { onFlashHidden(); });
    addChild(fc, 3);

    // Brief countdown "3-2-1-BLINK" then start
    blinkLabel->setVisible(false);

    scheduleOnce([this, fc, blinkLabel](float) {
        blinkLabel->setVisible(true);
        scheduleOnce([this, fc, blinkLabel](float) {
            blinkLabel->setVisible(false);
            fc->start();
        }, 0.3f, "start_flash");
    }, 0.5f, "pre_flash");

    return true;
}

void FlashLayer::onFlashHidden() {
    // Transition to level selection
    auto* sel = LevelSelectionLayer::create(m_fragment);
    if (!sel) return;
    CCDirector::sharedDirector()->replaceScene(
        CCTransitionFade::create(0.3f, sel)
    );
}
