#pragma once
#include <Geode/Geode.hpp>
#include "../levels/LevelData.hpp"

namespace blinky {

// Shows the selected level in full, lets the player scroll and click
// to guess the location of the original fragment.
class PreviewLayer : public cocos2d::CCLayer {
public:
    static PreviewLayer* create(
        const LevelFragment& fragment,
        int selectedLevelID);

    bool init(const LevelFragment& fragment, int selectedLevelID);

    // Touch handling for drag-scroll and tap-to-guess
    bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
    void ccTouchMoved(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
    void ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;

private:
    void onConfirmGuess(cocos2d::CCObject* sender);
    void showGuessMarker(float levelX, float levelY);

    LevelFragment m_fragment;
    int           m_selectedLevelID;

    cocos2d::CCNode* m_levelNode     = nullptr;  // the rendered level
    cocos2d::CCNode* m_guessMarker   = nullptr;
    cocos2d::CCNode* m_confirmButton = nullptr;

    // Scroll state
    cocos2d::CCPoint m_lastTouchPos;
    bool             m_dragging    = false;
    float            m_scrollX     = 0.f;
    float            m_minScrollX  = 0.f;
    float            m_maxScrollX  = 0.f;

    // Guess state
    bool  m_guessLocked = false;
    float m_guessLevelX = 0.f;
    float m_guessLevelY = 0.f;

    // Layout constants
    float m_pixelsPerUnit = 0.5f;  // output pixels per level unit
    float m_viewportW     = 0.f;
    float m_viewportH     = 0.f;
    float m_viewportOriginX = 0.f;
    float m_viewportOriginY = 0.f;

    // Convert screen tap → level coordinates
    cocos2d::CCPoint screenToLevel(cocos2d::CCPoint screenPt) const;
};

} // namespace blinky
