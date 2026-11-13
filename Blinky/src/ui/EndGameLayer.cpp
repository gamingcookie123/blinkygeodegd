#include "EndGameLayer.hpp"
#include "BlinkyMenu.hpp"
#include "FlashLayer.hpp"
#include "../game/GameState.hpp"
#include "../game/FragmentGenerator.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/MenuLayer.hpp>
#include <algorithm>
#include <numeric>
#include <string>
#include <climits>

using namespace blinky;
using namespace geode::prelude;
using namespace cocos2d;

EndGameLayer* EndGameLayer::create() {
    auto* obj = new EndGameLayer();
    if (obj && obj->init()) {
        obj->autorelease();
        return obj;
    }
    CC_SAFE_DELETE(obj);
    return nullptr;
}

bool EndGameLayer::init() {
    if (!CCLayer::init()) return false;

    auto& state  = gState();
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    float cx     = winSize.width * 0.5f;

    // Background gradient effect
    auto* bg = CCLayerGradient::create(
        { 5, 10, 30, 255 },
        { 15, 20, 50, 255 });
    addChild(bg, 0);

    // ---- Compute stats ----
    int totalPenalty   = state.totalPenalty;
    int roundCount     = static_cast<int>(state.results.size());
    int correctLevels  = 0;
    int perfectRounds  = 0;
    int bestPenalty    = INT_MAX;
    int worstPenalty   = INT_MIN;

    for (const auto& r : state.results) {
        if (r.correctLevel) correctLevels++;
        if (r.penaltyBlocks == 0) perfectRounds++;
        if (r.penaltyBlocks < bestPenalty)  bestPenalty  = r.penaltyBlocks;
        if (r.penaltyBlocks > worstPenalty) worstPenalty = r.penaltyBlocks;
    }
    if (bestPenalty  == INT_MAX) bestPenalty  = 0;
    if (worstPenalty == INT_MIN) worstPenalty = 0;
    float avgPenalty = roundCount > 0
        ? static_cast<float>(totalPenalty) / roundCount
        : 0.f;

    // ---- Panel ----
    float panelW = winSize.width  * 0.82f;
    float panelH = winSize.height * 0.80f;
    auto* panel = CCScale9Sprite::create("GJ_square07.png");
    panel->setContentSize({ panelW, panelH });
    panel->setPosition({ cx, winSize.height * 0.5f + 14.f });
    addChild(panel, 1);

    float y = winSize.height * 0.5f + 14.f + panelH * 0.5f - 30.f;

    // Title
    auto* title = CCLabelBMFont::create("Game Over!", "bigFont.fnt");
    title->setScale(0.80f);
    title->setColor({ 255, 220, 50 });
    title->setPosition({ cx, y });
    addChild(title, 2);
    y -= 36.f;

    // Rank label
    std::string rankStr;
    CCColor3B rankColor;
    if (avgPenalty == 0.f)      { rankStr = "PERFECT RUN!"; rankColor = { 50, 255, 100 }; }
    else if (avgPenalty <= 5.f) { rankStr = "Expert";       rankColor = { 100, 220, 255 }; }
    else if (avgPenalty <= 15.f){ rankStr = "Great";        rankColor = { 100, 255, 100 }; }
    else if (avgPenalty <= 30.f){ rankStr = "Good";         rankColor = { 255, 220, 50 }; }
    else if (avgPenalty <= 60.f){ rankStr = "Fair";         rankColor = { 255, 160, 50 }; }
    else                        { rankStr = "Needs Practice"; rankColor = { 255, 100, 80 }; }

    auto* rankLabel = CCLabelBMFont::create(rankStr.c_str(), "bigFont.fnt");
    rankLabel->setScale(0.60f);
    rankLabel->setColor(rankColor);
    rankLabel->setPosition({ cx, y });
    addChild(rankLabel, 2);
    y -= 30.f;

    // Divider
    auto addDiv = [&]() {
        auto* div = CCDrawNode::create();
        div->drawSegment({ cx - panelW * 0.4f, y },
                         { cx + panelW * 0.4f, y }, 1.f, { 0.5f, 0.5f, 0.6f, 0.5f });
        addChild(div, 2);
        y -= 16.f;
    };
    addDiv();

    // Stats rows helper
    auto addStat = [&](const std::string& label, const std::string& value, CCColor3B vc) {
        auto* lbl = CCLabelBMFont::create(label.c_str(), "goldFont.fnt");
        lbl->setScale(0.40f);
        lbl->setColor({ 150, 150, 170 });
        lbl->setPosition({ cx - panelW * 0.22f, y });
        addChild(lbl, 2);

        auto* val = CCLabelBMFont::create(value.c_str(), "bigFont.fnt");
        val->setScale(0.46f);
        val->setColor(vc);
        val->setPosition({ cx + panelW * 0.14f, y });
        addChild(val, 2);
        y -= 24.f;
    };

    // Format helpers
    auto fmtInt = [](int v) { return std::to_string(v); };
    auto fmtFloat = [](float v) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.1f", v);
        return std::string(buf);
    };

    addStat("Total penalty:",    fmtInt(totalPenalty) + " blocks",          { 255, 200, 100 });
    addStat("Avg per round:",    fmtFloat(avgPenalty) + " blocks",          { 255, 220, 50 });
    addDiv();
    addStat("Rounds played:",    fmtInt(roundCount),                        { 200, 200, 255 });
    addStat("Best round:",       fmtInt(bestPenalty) + " blocks",           { 50, 220, 100 });
    addStat("Worst round:",      fmtInt(worstPenalty) + " blocks",          { 255, 100, 80 });
    addDiv();
    addStat("Correct levels:",
        fmtInt(correctLevels) + " / " + fmtInt(roundCount),
        correctLevels == roundCount ? CCColor3B{ 50, 220, 100 } : CCColor3B{ 255, 200, 100 });
    addStat("Perfect rounds:",
        fmtInt(perfectRounds),
        perfectRounds > 0 ? CCColor3B{ 50, 255, 150 } : CCColor3B{ 200, 200, 200 });

    // ---- Buttons ----
    auto* menu = CCMenu::create();
    menu->setPosition({ cx, 36.f });
    menu->setLayout(RowLayout::create()
        ->setGap(14.f)
        ->setAxisAlignment(AxisAlignment::Center));
    addChild(menu, 3);

    auto makeBtn = [](const char* text, float w, const char* sprite) {
        auto* bg = CCScale9Sprite::create(sprite);
        bg->setContentSize({ w, 44.f });
        auto* lbl = CCLabelBMFont::create(text, "bigFont.fnt");
        lbl->setScale(0.40f);
        lbl->setPosition({ w * 0.5f, 22.f });
        bg->addChild(lbl);
        return bg;
    };

    auto* playAgainItem = CCMenuItemSpriteExtra::create(
        makeBtn("Play Again", 150.f, "GJ_button_01.png"),
        nullptr, this, menu_selector(EndGameLayer::onPlayAgain));
    menu->addChild(playAgainItem);

    auto* blinkyItem = CCMenuItemSpriteExtra::create(
        makeBtn("Blinky Menu", 150.f, "GJ_button_04.png"),
        nullptr, this, menu_selector(EndGameLayer::onBlinkyMenu));
    menu->addChild(blinkyItem);

    auto* mainItem = CCMenuItemSpriteExtra::create(
        makeBtn("Main Menu", 130.f, "GJ_button_06.png"),
        nullptr, this, menu_selector(EndGameLayer::onMainMenu));
    menu->addChild(mainItem);

    menu->updateLayout();
    return true;
}

void EndGameLayer::onPlayAgain(CCObject*) {
    gState().reset();
    LevelFragment frag;
    if (!FragmentGenerator::get().generate(frag)) return;
    gState().currentFragment = frag;
    auto* flash = FlashLayer::create(frag);
    CCDirector::sharedDirector()->replaceScene(
        CCTransitionFade::create(0.3f, flash));
}

void EndGameLayer::onBlinkyMenu(CCObject*) {
    gState().reset();
    auto* bm = BlinkyMenu::create();
    CCDirector::sharedDirector()->replaceScene(
        CCTransitionFade::create(0.3f, bm));
}

void EndGameLayer::onMainMenu(CCObject*) {
    gState().reset();
    auto* ml = MenuLayer::scene(false);
    CCDirector::sharedDirector()->replaceScene(
        CCTransitionFade::create(0.4f, ml));
}
