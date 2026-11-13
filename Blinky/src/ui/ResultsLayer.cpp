#include "ResultsLayer.hpp"
#include "FlashLayer.hpp"
#include "EndGameLayer.hpp"
#include "../game/GameState.hpp"
#include "../game/FragmentGenerator.hpp"
#include "../levels/LevelDatabase.hpp"
#include <Geode/Geode.hpp>
#include <cmath>
#include <string>

using namespace blinky;
using namespace geode::prelude;
using namespace cocos2d;

ResultsLayer* ResultsLayer::create(const RoundResult& result) {
    auto* obj = new ResultsLayer();
    if (obj && obj->init(result)) {
        obj->autorelease();
        return obj;
    }
    CC_SAFE_DELETE(obj);
    return nullptr;
}

static std::string levelNameFor(int id) {
    for (const auto& info : LevelDatabase::get().getEligibleLevels())
        if (info.levelID == id) return info.name;
    return "Unknown (" + std::to_string(id) + ")";
}

bool ResultsLayer::init(const RoundResult& result) {
    if (!CCLayer::init()) return false;
    m_result = result;

    auto winSize = CCDirector::sharedDirector()->getWinSize();
    float cx = winSize.width * 0.5f;

    // Background
    auto* bg = CCLayerColor::create({ 10, 10, 20, 255 });
    addChild(bg, 0);

    // Panel
    float panelW = winSize.width  * 0.82f;
    float panelH = winSize.height * 0.76f;
    auto* panel = CCScale9Sprite::create("GJ_square07.png");
    panel->setContentSize({ panelW, panelH });
    panel->setPosition({ cx, winSize.height * 0.5f + 10.f });
    addChild(panel, 1);

    float panelTop = winSize.height * 0.5f + 10.f + panelH * 0.5f;
    float y = panelTop - 28.f;

    // --- Title ---
    auto* titleLabel = CCLabelBMFont::create("Round Result", "bigFont.fnt");
    titleLabel->setScale(0.65f);
    titleLabel->setColor({ 255, 220, 50 });
    titleLabel->setPosition({ cx, y });
    addChild(titleLabel, 2);
    y -= 30.f;

    // --- Penalty ---
    std::string penStr = "Penalty: " + std::to_string(result.penaltyBlocks) + " blocks";
    CCColor3B penColor = result.penaltyBlocks == 0
        ? CCColor3B{ 50, 255, 100 }
        : (result.penaltyBlocks <= 10
            ? CCColor3B{ 255, 220, 50 }
            : CCColor3B{ 255, 100, 80 });

    auto* penLabel = CCLabelBMFont::create(penStr.c_str(), "bigFont.fnt");
    penLabel->setScale(0.75f);
    penLabel->setColor(penColor);
    penLabel->setPosition({ cx, y });
    addChild(penLabel, 2);
    y -= 28.f;

    // Perfect round text
    if (result.penaltyBlocks == 0) {
        auto* perfect = CCLabelBMFont::create("PERFECT!", "bigFont.fnt");
        perfect->setScale(0.55f);
        perfect->setColor({ 100, 255, 150 });
        perfect->setPosition({ cx, y });
        addChild(perfect, 2);
        y -= 24.f;
    }

    // Divider
    auto* div1 = CCDrawNode::create();
    div1->drawSegment({ cx - panelW * 0.4f, y },
                      { cx + panelW * 0.4f, y }, 1.f, { 0.5f, 0.5f, 0.6f, 0.5f });
    addChild(div1, 2);
    y -= 18.f;

    // --- Level identification ---
    auto addRow = [&](const std::string& lbl, const std::string& val, CCColor3B col) {
        auto* rowLbl = CCLabelBMFont::create(lbl.c_str(), "goldFont.fnt");
        rowLbl->setScale(0.38f);
        rowLbl->setColor({ 160, 160, 180 });
        rowLbl->setPosition({ cx - panelW * 0.25f, y });
        addChild(rowLbl, 2);

        auto* rowVal = CCLabelBMFont::create(val.c_str(), "bigFont.fnt");
        rowVal->setScale(0.40f);
        rowVal->setColor(col);
        rowVal->setPosition({ cx + panelW * 0.1f, y });
        rowVal->limitLabelWidth(panelW * 0.44f, 0.40f, 0.1f);
        addChild(rowVal, 2);
        y -= 22.f;
    };

    addRow("Correct level:",
           levelNameFor(result.targetLevelID),
           { 50, 220, 100 });

    CCColor3B guessLvlColor = result.correctLevel
        ? CCColor3B{ 50, 220, 100 }
        : CCColor3B{ 255, 100, 80 };
    addRow("Your level:",
           levelNameFor(result.guessedLevelID),
           guessLvlColor);

    if (!result.correctLevel) {
        bool penEnabled = Mod::get()->getSettingValue<bool>("wrong-level-penalty");
        if (penEnabled) {
            addRow("Wrong level penalty:", "+50 blocks", { 255, 80, 80 });
        } else {
            auto* note = CCLabelBMFont::create("(Wrong level - no extra penalty in settings)",
                                               "goldFont.fnt");
            note->setScale(0.30f);
            note->setColor({ 200, 150, 100 });
            note->setPosition({ cx, y });
            addChild(note, 2);
            y -= 18.f;
        }
    }

    y -= 6.f;
    auto* div2 = CCDrawNode::create();
    div2->drawSegment({ cx - panelW * 0.4f, y },
                      { cx + panelW * 0.4f, y }, 1.f, { 0.5f, 0.5f, 0.6f, 0.5f });
    addChild(div2, 2);
    y -= 18.f;

    // --- Position data ---
    auto fmtPos = [](float x, float y) {
        return "(" + std::to_string(static_cast<int>(x / 30.f))
             + ", " + std::to_string(static_cast<int>(y / 30.f)) + ") blocks";
    };
    addRow("Target position:",    fmtPos(result.targetX,  result.targetY),  { 50, 220, 100 });
    addRow("Your guess:",         fmtPos(result.guessedX, result.guessedY), { 200, 200, 255 });

    std::string distStr = std::to_string(static_cast<int>(
        std::round(result.distanceUnits / 30.f))) + " blocks away";
    addRow("Distance:", distStr, { 255, 200, 100 });

    y -= 10.f;
    auto* div3 = CCDrawNode::create();
    div3->drawSegment({ cx - panelW * 0.4f, y },
                      { cx + panelW * 0.4f, y }, 1.f, { 0.5f, 0.5f, 0.6f, 0.5f });
    addChild(div3, 2);
    y -= 18.f;

    // --- Total penalty so far ---
    std::string totalStr = "Total penalty: " + std::to_string(gState().totalPenalty) + " blocks";
    auto* totalLabel = CCLabelBMFont::create(totalStr.c_str(), "bigFont.fnt");
    totalLabel->setScale(0.48f);
    totalLabel->setColor({ 255, 220, 50 });
    totalLabel->setPosition({ cx, y });
    addChild(totalLabel, 2);

    // ---- Buttons ----
    bool gameOver = gState().isOver();

    auto* menu = CCMenu::create();
    menu->setPosition({ cx, 30.f });
    addChild(menu, 3);

    if (!gameOver) {
        // Next round button
        auto* nextBg = CCScale9Sprite::create("GJ_button_01.png");
        nextBg->setContentSize({ 180.f, 48.f });
        auto* nextLbl = CCLabelBMFont::create("Next Round", "bigFont.fnt");
        nextLbl->setScale(0.5f);
        nextLbl->setPosition({ 90.f, 24.f });
        nextBg->addChild(nextLbl);

        auto* nextBtn = CCMenuItemSpriteExtra::create(
            nextBg, nullptr, this,
            menu_selector(ResultsLayer::onNextRound));
        nextBtn->setPosition({ 0, 0 });
        menu->addChild(nextBtn);
    } else {
        // End game button
        auto* endBg = CCScale9Sprite::create("GJ_button_02.png");
        endBg->setContentSize({ 200.f, 48.f });
        auto* endLbl = CCLabelBMFont::create("See Final Score", "bigFont.fnt");
        endLbl->setScale(0.48f);
        endLbl->setPosition({ 100.f, 24.f });
        endBg->addChild(endLbl);

        auto* endBtn = CCMenuItemSpriteExtra::create(
            endBg, nullptr, this,
            menu_selector(ResultsLayer::onEndGame));
        endBtn->setPosition({ 0, 0 });
        menu->addChild(endBtn);
    }

    return true;
}

void ResultsLayer::onNextRound(CCObject*) {
    LevelFragment frag;
    if (!FragmentGenerator::get().generate(frag)) {
        auto* err = CCLabelBMFont::create("No fragment available. Check settings.", "bigFont.fnt");
        err->setScale(0.4f);
        err->setPosition({
            CCDirector::sharedDirector()->getWinSize().width * 0.5f,
            CCDirector::sharedDirector()->getWinSize().height * 0.5f
        });
        addChild(err, 10);
        return;
    }
    gState().currentFragment = frag;
    auto* flash = FlashLayer::create(frag);
    CCDirector::sharedDirector()->replaceScene(
        CCTransitionFade::create(0.3f, flash));
}

void ResultsLayer::onEndGame(CCObject*) {
    auto* end = EndGameLayer::create();
    CCDirector::sharedDirector()->replaceScene(
        CCTransitionFade::create(0.4f, end));
}
