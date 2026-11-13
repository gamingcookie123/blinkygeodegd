#pragma once
#include <Geode/Geode.hpp>
#include <functional>

namespace blinky {

// Handles the frame-accurate flash timing.
// Because Cocos2d is frame-based (≈16.67 ms/frame at 60 FPS), the
// minimum reliable display duration is one frame.  The configured
// flash-duration-ms setting is rounded up to the nearest frame boundary.
class FlashController : public cocos2d::CCNode {
public:
    static FlashController* create(
        cocos2d::CCNode* fragmentNode,
        std::function<void()> onHidden
    );

    // Start counting frames; hides fragmentNode when done and calls onHidden.
    void start();

private:
    bool init(cocos2d::CCNode* fragmentNode, std::function<void()> onHidden);
    void update(float dt) override;

    cocos2d::CCNode*      m_fragment  = nullptr;
    std::function<void()> m_callback;
    int                   m_targetFrames = 1;
    int                   m_frameCount   = 0;
    bool                  m_running      = false;
};

} // namespace blinky
