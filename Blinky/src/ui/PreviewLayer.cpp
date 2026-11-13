#include "PreviewLayer.hpp"
#include "ResultsLayer.hpp"
#include "../levels/LevelDatabase.hpp"
#include "../levels/LevelLoader.hpp"
#include "../levels/LevelRenderer.hpp"
#include "../game/ScoreCalculator.hpp"
#include "../game/GameState.hpp"
#include <Geode/Geode.hpp>

using namespace blinky;
using namespace geode::prelude;
using namespace cocos2d;

PreviewLayer* PreviewLayer::create(
    const LevelFragment& fragment, int selectedLevelID)
{
    auto* obj = new PreviewLayer();
    if (obj && obj->init(fragment, selectedLevelID)) {
        obj->autorelease();
        return obj;
    }
    CC_SAFE_DELETE(obj);
    return nullptr;
}

bool PreviewLayer::init(const LevelFragment& fragment, int selectedLevelID) {
    if (!CCLayer::init()) return false;
    m_fragment        = fragment;
    m_selectedLevelID = selectedLevelID;

    setTouchEnabled(true);
    setTouchMode(kCCTouchesOneByOne);

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    // Dark BG
    auto* bg = CCLayerColor::create({ 10, 10, 20, 255 });
    addChild(bg, 0);

    // Header bar
    auto* header = CCLayerColor::create({ 20, 20, 40, 230 },
        winSize.width, 55.f);
    header->setPosition({ 0, winSize.height - 55.f });
    addChild(header, 5);

    // Level name in header
    std::string levelName = "Unknown Level";
    for (const auto& info : LevelDatabase::get().getEligibleLevels()) {
        if (info.levelID == selectedLevelID) { levelName = info.name; break; }
    }
    auto* nameLabel = CCLabelBMFont::create(levelName.c_str(), "bigFont.fnt");
    nameLabel->setScale(0.5f);
    nameLabel->setPosition({ winSize.width * 0.5f, winSize.height - 28.f });
    addChild(nameLabel, 6);

    // Instruction label
    auto* instrLabel = CCLabelBMFont::create(
        "Tap to place your guess  •  Drag to scroll", "goldFont.fnt");
    instrLabel->setScale(0.35f);
    instrLabel->setColor({ 200, 200, 200 });
    instrLabel->setPosition({ winSize.width * 0.5f, 18.f });
    addChild(instrLabel, 5);

    // Footer bar
    auto* footer = CCLayerColor::create({ 20, 20, 40, 230 },
        winSize.width, 36.f);
    footer->setPosition({ 0, 0 });
    addChild(footer, 5);

    // ---- Render the selected level ----
    auto& db  = LevelDatabase::get();
    auto& ldr = LevelLoader::get();
    auto& rnd = LevelRenderer::get();

    GJGameLevel* gj = db.getGJLevel(selectedLevelID);
    ParsedLevel parsed;
    bool loaded = gj && ldr.load(gj, parsed);

    // Viewport for the level preview (between header and footer)
    m_viewportOriginX = 0.f;
    m_viewportOriginY = 36.f;
    m_viewportW = winSize.width;
    m_viewportH = winSize.height - 55.f - 36.f;

    if (!loaded) {
        auto* err = CCLabelBMFont::create("Error loading level", "bigFont.fnt");
        err->setScale(0.5f);
        err->setPosition({ winSize.width * 0.5f, winSize.height * 0.5f });
        addChild(err, 6);
    } else {
        // pixelsPerUnit: fit level height into viewport height
        // GD standard level height ≈ 480 units; we render that into viewportH
        m_pixelsPerUnit = m_viewportH / 480.f;

        m_levelNode = rnd.renderFullLevel(parsed, m_pixelsPerUnit);
        if (m_levelNode) {
            auto levelSize = m_levelNode->getContentSize();

            // Clip region: CCClippingNode to viewport bounds
            auto* stencil = CCDrawNode::create();
            stencil->drawSolidRect(
                { 0, 0 },
                { m_viewportW, m_viewportH },
                { 1, 1, 1, 1 }
            );
            auto* clipper = CCClippingNode::create(stencil);
            clipper->setAlphaThreshold(0.05f);
            clipper->setPosition({ m_viewportOriginX, m_viewportOriginY });
            clipper->addChild(m_levelNode);
            addChild(clipper, 2);

            // Position level node at start (left edge)
            m_levelNode->setPosition({ 0, 0 });
            m_scrollX = 0.f;

            // Scroll limits
            m_minScrollX = -(levelSize.width - m_viewportW);
            if (m_minScrollX > 0) m_minScrollX = 0;
            m_maxScrollX = 0.f;
        }
    }

    // Confirm button (hidden until guess placed)
    auto* confirmBg = CCScale9Sprite::create("GJ_button_02.png");
    confirmBg->setContentSize({ 160.f, 44.f });
    auto* confirmLabel = CCLabelBMFont::create("Confirm Guess", "bigFont.fnt");
    confirmLabel->setScale(0.42f);
    confirmLabel->setPosition({ 80.f, 22.f });
    confirmBg->addChild(confirmLabel);

    auto* confirmItem = CCMenuItemSpriteExtra::create(
        confirmBg, nullptr, this,
        menu_selector(PreviewLayer::onConfirmGuess));
    confirmItem->setVisible(false);
    m_confirmButton = confirmItem;

    auto* confirmMenu = CCMenu::create(confirmItem, nullptr);
    confirmMenu->setPosition({
        winSize.width * 0.5f,
        m_viewportOriginY * 0.5f + 2.f
    });
    addChild(confirmMenu, 6);

    // Guess marker node (crosshair)
    m_guessMarker = CCNode::create();
    auto* draw = CCDrawNode::create();
    // Horizontal line
    draw->drawSegment({ -16, 0 }, { 16, 0 }, 1.5f, { 1, 0, 0, 1 });
    // Vertical line
    draw->drawSegment({ 0, -16 }, { 0, 16 }, 1.5f, { 1, 0, 0, 1 });
    // Circle
    draw->drawDot({ 0, 0 }, 5.f, { 1, 0.2f, 0.2f, 1 });
    m_guessMarker->addChild(draw);
    m_guessMarker->setVisible(false);
    addChild(m_guessMarker, 8);

    return true;
}

// ---- Coordinate conversion ----

CCPoint PreviewLayer::screenToLevel(CCPoint screenPt) const {
    // screenPt is in window coordinates (bottom-left = 0,0)
    // Convert to level units
    // The level node's x position tracks the scroll offset
    float nodeX = 0.f;
    if (m_levelNode) nodeX = m_levelNode->getPositionX();

    // local position within viewport
    float localX = screenPt.x - m_viewportOriginX - nodeX;
    float localY = screenPt.y - m_viewportOriginY;

    // Convert to level units
    float levelX = localX / m_pixelsPerUnit;
    float levelY = localY / m_pixelsPerUnit;

    // The renderFullLevel uses originY = -150 in level space; adjust
    // See LevelRenderer::renderFullLevel: originY = -150, height = 600
    levelY = levelY + (-150.f);  // subtract the render origin

    return { levelX, levelY };
}

// ---- Touch handling ----

bool PreviewLayer::ccTouchBegan(CCTouch* touch, CCEvent*) {
    m_dragging = false;
    m_lastTouchPos = touch->getLocation();
    return true;
}

void PreviewLayer::ccTouchMoved(CCTouch* touch, CCEvent*) {
    if (m_guessLocked) return;
    CCPoint current = touch->getLocation();
    float dx = current.x - m_lastTouchPos.x;
    m_dragging = true;

    // Scroll the level
    if (m_levelNode) {
        m_scrollX += dx;
        m_scrollX = std::max(m_minScrollX, std::min(m_maxScrollX, m_scrollX));
        m_levelNode->setPositionX(m_scrollX);
    }
    m_lastTouchPos = current;
}

void PreviewLayer::ccTouchEnded(CCTouch* touch, CCEvent*) {
    if (m_guessLocked) return;
    if (m_dragging) return; // was a scroll, not a tap

    // Check touch is within viewport
    CCPoint loc = touch->getLocation();
    if (loc.x < m_viewportOriginX || loc.x > m_viewportOriginX + m_viewportW) return;
    if (loc.y < m_viewportOriginY || loc.y > m_viewportOriginY + m_viewportH) return;

    CCPoint levelCoords = screenToLevel(loc);
    m_guessLevelX = levelCoords.x;
    m_guessLevelY = levelCoords.y;

    showGuessMarker(levelCoords.x, levelCoords.y);
    if (m_confirmButton) m_confirmButton->setVisible(true);
}

void PreviewLayer::showGuessMarker(float levelX, float levelY) {
    if (!m_guessMarker) return;
    // Convert back to screen coords
    float nodeX = m_levelNode ? m_levelNode->getPositionX() : 0.f;
    float screenX = levelX * m_pixelsPerUnit + nodeX + m_viewportOriginX;
    float screenY = (levelY - (-150.f)) * m_pixelsPerUnit + m_viewportOriginY;

    m_guessMarker->setPosition({ screenX, screenY });
    m_guessMarker->setVisible(true);
}

void PreviewLayer::onConfirmGuess(CCObject*) {
    if (m_guessLocked) return;
    m_guessLocked = true;

    // Build result
    auto result = ScoreCalculator::buildResult(
        m_fragment, m_selectedLevelID, m_guessLevelX, m_guessLevelY);

    // Go to results layer
    auto* resultsLayer = ResultsLayer::create(result);
    CCDirector::sharedDirector()->replaceScene(
        CCTransitionFade::create(0.3f, resultsLayer));
}
