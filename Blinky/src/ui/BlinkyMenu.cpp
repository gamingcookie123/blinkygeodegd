#include "BlinkyMenu.hpp"
#include "FlashLayer.hpp"
#include "../game/FragmentGenerator.hpp"
#include "../game/GameState.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/MenuLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>

using namespace blinky;
using namespace geode::prelude;
using namespace cocos2d;

BlinkyMenu* BlinkyMenu::create() {
    auto* obj = new BlinkyMenu();
    if (obj && obj->init()) {
        obj->autorelease();
        return obj;
    }
    CC_SAFE_DELETE(obj);
    return nullptr;
}

bool BlinkyMenu::init() {
    if (!CCLayer::init()) return false;

    m_modeButtons = CCArray::create();
    m_modeButtons->retain();

    auto winSize = CCDirector::sharedDirector()->getWinSize();
    float cx = winSize.width * 0.5f;
    float cy = winSize.height * 0.5f;

    // ---- Background ----
    auto* bg = CCLayerGradient::create(
        { 5, 10, 30, 255 },
        { 15, 25, 60, 255 });
    addChild(bg, 0);

    // Subtle star-like dots
    for (int i = 0; i < 40; i++) {
        auto* dot = CCDrawNode::create();
        float px = static_cast<float>(rand() % static_cast<int>(winSize.width));
        float py = static_cast<float>(rand() % static_cast<int>(winSize.height));
        float sz = 1.0f + (rand() % 100) / 100.f;
        dot->drawDot({ 0, 0 }, sz, { 1.f, 1.f, 1.f, 0.3f });
        dot->setPosition({ px, py });
        addChild(dot, 0);
    }

    // ---- Title ----
    auto* titleShadow = CCLabelBMFont::create("BLINKY", "bigFont.fnt");
    titleShadow->setScale(1.1f);
    titleShadow->setColor({ 50, 20, 100 });
    titleShadow->setOpacity(180);
    titleShadow->setPosition({ cx + 3.f, winSize.height - 55.f - 3.f });
    addChild(titleShadow, 1);

    auto* title = CCLabelBMFont::create("BLINKY", "bigFont.fnt");
    title->setScale(1.1f);
    title->setColor({ 255, 220, 50 });
    title->setPosition({ cx, winSize.height - 55.f });
    addChild(title, 2);

    // Pulsing animation on title
    title->runAction(CCRepeatForever::create(
        CCSequence::create(
            CCScaleTo::create(1.2f, 1.15f),
            CCScaleTo::create(1.2f, 1.05f),
            nullptr)));

    // Subtitle
    auto* sub = CCLabelBMFont::create("See it. Find it. Guess it.", "goldFont.fnt");
    sub->setScale(0.42f);
    sub->setColor({ 180, 180, 220 });
    sub->setPosition({ cx, winSize.height - 82.f });
    addChild(sub, 2);

    // ---- Mode selection ----
    auto* modeTitle = CCLabelBMFont::create("SELECT MODE", "goldFont.fnt");
    modeTitle->setScale(0.42f);
    modeTitle->setColor({ 160, 160, 200 });
    modeTitle->setPosition({ cx, cy + 55.f });
    addChild(modeTitle, 2);

    struct ModeInfo { const char* name; GameMode mode; int tag; };
    std::vector<ModeInfo> modes = {
        { "5 Rounds",   GameMode::Rounds5,  0 },
        { "10 Rounds",  GameMode::Rounds10, 1 },
        { "Endless",    GameMode::Endless,  2 },
    };

    float modeSpacing = 90.f;
    float modeStartX  = cx - modeSpacing;

    auto* modeMenu = CCMenu::create();
    modeMenu->setPosition({ 0, 0 });
    addChild(modeMenu, 2);

    for (auto& mi : modes) {
        auto* btnBg = CCScale9Sprite::create("GJ_button_04.png");
        btnBg->setContentSize({ 80.f, 54.f });

        auto* lbl = CCLabelBMFont::create(mi.name, "bigFont.fnt");
        lbl->setScale(0.38f);
        lbl->setPosition({ 40.f, 27.f });
        btnBg->addChild(lbl);

        auto* btn = CCMenuItemSpriteExtra::create(
            btnBg, nullptr, this,
            menu_selector(BlinkyMenu::onPlayMode));
        btn->setTag(mi.tag);
        btn->setPosition({
            modeStartX + mi.tag * modeSpacing,
            cy + 24.f
        });
        modeMenu->addChild(btn);
        m_modeButtons->addObject(btn);
    }

    updateModeHighlight();

    // ---- Play button ----
    auto* playBg = CCScale9Sprite::create("GJ_button_01.png");
    playBg->setContentSize({ 200.f, 60.f });
    auto* playLbl = CCLabelBMFont::create("PLAY", "bigFont.fnt");
    playLbl->setScale(0.72f);
    playLbl->setPosition({ 100.f, 30.f });
    playBg->addChild(playLbl);

    auto* playItem = CCMenuItemSpriteExtra::create(
        playBg, nullptr, this,
        menu_selector(BlinkyMenu::onPlayMode)); // tag=-1 means "start"
    playItem->setTag(99); // 99 = "start game"
    playItem->setPosition({ cx, cy - 30.f });
    modeMenu->addChild(playItem);

    // ---- Settings button ----
    auto* settingsBg = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
    if (!settingsBg) settingsBg = CCSprite::create();
    auto* settingsItem = CCMenuItemSpriteExtra::create(
        settingsBg, nullptr, this,
        menu_selector(BlinkyMenu::onSettings));
    settingsItem->setPosition({ cx + 120.f, cy - 30.f });
    modeMenu->addChild(settingsItem);

    // ---- Back button ----
    auto* backBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"),
        nullptr, this,
        menu_selector(BlinkyMenu::onBack));
    backBtn->setPosition({ 25.f, winSize.height - 25.f });
    modeMenu->addChild(backBtn);

    // ---- How to play label ----
    auto* howto = CCLabelBMFont::create(
        "Flash  \xE2\x80\xA2  Identify  \xE2\x80\xA2  Explore  \xE2\x80\xA2  Guess  \xE2\x80\xA2  Score",
        "goldFont.fnt");
    howto->setScale(0.35f);
    howto->setColor({ 140, 140, 180 });
    howto->setPosition({ cx, 20.f });
    addChild(howto, 2);

    return true;
}

void BlinkyMenu::updateModeHighlight() {
    int activeTag = (m_selectedMode == GameMode::Rounds5)  ? 0
                  : (m_selectedMode == GameMode::Rounds10) ? 1 : 2;

    for (int i = 0; i < m_modeButtons->count(); i++) {
        auto* btn = static_cast<CCNode*>(m_modeButtons->objectAtIndex(i));
        auto* bg  = dynamic_cast<CCScale9Sprite*>(
            btn->getChildren()->objectAtIndex(0));
        if (bg) {
            bg->setColor(btn->getTag() == activeTag
                ? CCColor3B{ 255, 220, 50 }
                : CCColor3B{ 255, 255, 255 });
        }
    }
}

void BlinkyMenu::onPlayMode(CCObject* sender) {
    int tag = static_cast<CCNode*>(sender)->getTag();

    if (tag == 99) {
        // Start game
        gState().reset();
        gState().mode = m_selectedMode;

        LevelFragment frag;
        if (!FragmentGenerator::get().generate(frag)) {
            // Show error
            auto* msg = CCLabelBMFont::create(
                "No levels available! Check Settings.", "bigFont.fnt");
            msg->setScale(0.4f);
            msg->setPosition({ CCDirector::sharedDirector()->getWinSize().width * 0.5f,
                               CCDirector::sharedDirector()->getWinSize().height * 0.3f });
            addChild(msg, 10);
            return;
        }
        gState().currentFragment = frag;
        auto* flash = FlashLayer::create(frag);
        CCDirector::sharedDirector()->replaceScene(
            CCTransitionFade::create(0.3f, flash));
        return;
    }

    // Mode selection
    if (tag == 0) m_selectedMode = GameMode::Rounds5;
    if (tag == 1) m_selectedMode = GameMode::Rounds10;
    if (tag == 2) m_selectedMode = GameMode::Endless;
    updateModeHighlight();
}

void BlinkyMenu::onSettings(CCObject*) {
    openSettingsPopup(Mod::get());
}

void BlinkyMenu::onBack(CCObject*) {
    auto* ml = MenuLayer::scene(false);
    CCDirector::sharedDirector()->replaceScene(
        CCTransitionFade::create(0.3f, ml));
}
