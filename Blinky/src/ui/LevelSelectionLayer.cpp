#include "LevelSelectionLayer.hpp"
#include "PreviewLayer.hpp"
#include "../levels/LevelDatabase.hpp"
#include "../game/GameState.hpp"
#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>

using namespace blinky;
using namespace geode::prelude;
using namespace cocos2d;
using namespace cocos2d::extension;

LevelSelectionLayer* LevelSelectionLayer::create(const LevelFragment& fragment) {
    auto* obj = new LevelSelectionLayer();
    if (obj && obj->init(fragment)) {
        obj->autorelease();
        return obj;
    }
    CC_SAFE_DELETE(obj);
    return nullptr;
}

bool LevelSelectionLayer::init(const LevelFragment& fragment) {
    if (!CCLayer::init()) return false;
    m_fragment = fragment;
    m_allButtons = CCArray::create();
    m_allButtons->retain();

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    // Dark background
    auto* bg = CCLayerColor::create({ 10, 10, 20, 255 });
    addChild(bg, 0);

    // Title
    auto* title = CCLabelBMFont::create("Which level was that?", "bigFont.fnt");
    title->setScale(0.6f);
    title->setColor({ 255, 220, 50 });
    title->setPosition({ winSize.width * 0.5f, winSize.height - 25.f });
    addChild(title, 2);

    // Subtitle
    auto* sub = CCLabelBMFont::create("Select the level that contained the fragment", "goldFont.fnt");
    sub->setScale(0.4f);
    sub->setPosition({ winSize.width * 0.5f, winSize.height - 48.f });
    addChild(sub, 2);

    // Panel background for list
    float panelW = winSize.width  * 0.9f;
    float panelH = winSize.height * 0.72f;
    float panelX = (winSize.width - panelW) * 0.5f;
    float panelY = 60.f;

    auto* panelBg = CCScale9Sprite::create("GJ_square07.png");
    panelBg->setContentSize({ panelW, panelH });
    panelBg->setPosition({ winSize.width * 0.5f, panelY + panelH * 0.5f });
    addChild(panelBg, 1);

    // Build scrollable level list inside panel
    buildLevelList();

    return true;
}

void LevelSelectionLayer::buildLevelList() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    float panelW = winSize.width  * 0.9f;
    float panelH = winSize.height * 0.72f;
    float panelX = (winSize.width - panelW) * 0.5f;
    float panelY = 60.f;

    auto& db = LevelDatabase::get();
    auto levels = db.getEligibleLevels();

    // Grid layout: 2 columns
    float cellW = (panelW - 20.f) * 0.5f;
    float cellH = 44.f;
    float contentH = std::ceil(levels.size() / 2.f) * cellH + 10.f;
    float contentW = panelW - 20.f;

    auto* innerNode = CCNode::create();
    innerNode->setContentSize({ contentW, contentH });

    for (size_t i = 0; i < levels.size(); i++) {
        const auto& info = levels[i];

        int col = i % 2;
        int row = i / 2;
        float bx = col * cellW + cellW * 0.5f;
        float by = contentH - row * cellH - cellH * 0.5f;

        // Button background
        auto* btn = CCMenuItemSpriteExtra::create(
            CCScale9Sprite::create("GJ_button_01.png", { 0, 0, 40, 40 }),
            nullptr,
            this,
            menu_selector(LevelSelectionLayer::onLevelSelected)
        );

        auto* btnBg = CCScale9Sprite::create("GJ_button_01.png");
        btnBg->setContentSize({ cellW - 8.f, cellH - 8.f });

        auto* label = CCLabelBMFont::create(info.name.c_str(), "bigFont.fnt");
        label->setScale(0.38f);
        label->limitLabelWidth(cellW - 20.f, 0.38f, 0.1f);
        label->setPosition({ (cellW - 8.f) * 0.5f, (cellH - 8.f) * 0.5f });
        btnBg->addChild(label);

        auto* btnItem = CCMenuItemSpriteExtra::create(
            btnBg, nullptr, this,
            menu_selector(LevelSelectionLayer::onLevelSelected));
        btnItem->setTag(info.levelID);
        btnItem->setPosition({ bx, by });
        innerNode->addChild(btnItem);
        m_allButtons->addObject(btnItem);
    }

    // ScrollView
    auto* scrollView = CCScrollView::create({ panelW - 10.f, panelH - 10.f }, innerNode);
    scrollView->setPosition({ panelX + 5.f, panelY + 5.f });
    scrollView->setDirection(kCCScrollViewDirectionVertical);
    scrollView->setContentSize({ contentW, contentH });

    auto* menu = CCMenu::create();
    menu->setPosition({ 0, 0 });
    for (int i = 0; i < m_allButtons->count(); i++) {
        auto* item = static_cast<CCMenuItem*>(m_allButtons->objectAtIndex(i));
        menu->addChild(item);
    }
    innerNode->addChild(menu);

    addChild(scrollView, 2);
    m_scrollView = scrollView;
}

void LevelSelectionLayer::onLevelSelected(CCObject* sender) {
    auto* item = static_cast<CCNode*>(sender);
    int levelID = item->getTag();

    // Highlight selected button
    for (int i = 0; i < m_allButtons->count(); i++) {
        auto* btn = static_cast<CCNode*>(m_allButtons->objectAtIndex(i));
        auto* bg = dynamic_cast<CCScale9Sprite*>(btn->getChildren()->objectAtIndex(0));
        if (bg) {
            if (btn->getTag() == levelID)
                bg->setColor({ 255, 220, 50 });
            else
                bg->setColor({ 255, 255, 255 });
        }
    }

    m_selectedLevelID = levelID;

    // Transition to preview
    auto& db = LevelDatabase::get();
    std::string levelName;
    for (const auto& info : db.getEligibleLevels()) {
        if (info.levelID == levelID) { levelName = info.name; break; }
    }

    // Brief confirmation then go to preview
    auto* overlay = CCLayerColor::create({ 0, 0, 0, 0 });
    addChild(overlay, 10);
    overlay->runAction(CCSequence::create(
        CCFadeIn::create(0.15f),
        CCCallFunc::create(this, callfunc_selector(LevelSelectionLayer::goToPreview)),
        nullptr
    ));
}

void LevelSelectionLayer::goToPreview() {
    if (m_selectedLevelID < 0) return;
    auto* preview = PreviewLayer::create(m_fragment, m_selectedLevelID);
    if (!preview) return;
    CCDirector::sharedDirector()->replaceScene(
        CCTransitionFade::create(0.25f, preview));
}
