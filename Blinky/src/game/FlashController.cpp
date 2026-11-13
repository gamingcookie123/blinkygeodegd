#include "FlashController.hpp"
#include <Geode/Geode.hpp>
#include <cmath>

using namespace blinky;
using namespace geode::prelude;
using namespace cocos2d;

FlashController* FlashController::create(
    CCNode* fragmentNode,
    std::function<void()> onHidden)
{
    auto* obj = new FlashController();
    if (obj && obj->init(fragmentNode, std::move(onHidden))) {
        obj->autorelease();
        return obj;
    }
    CC_SAFE_DELETE(obj);
    return nullptr;
}

bool FlashController::init(CCNode* fragmentNode, std::function<void()> onHidden) {
    if (!CCNode::init()) return false;

    m_fragment = fragmentNode;
    m_callback = std::move(onHidden);

    // Convert ms setting → frames
    int durationMs = static_cast<int>(
        Mod::get()->getSettingValue<int64_t>("flash-duration-ms"));

    // Apply difficulty multiplier
    std::string diff = Mod::get()->getSettingValue<std::string>("difficulty");
    if (diff == "Easy")  durationMs = static_cast<int>(durationMs * 3.0f);
    if (diff == "Hard")  durationMs = std::max(17, static_cast<int>(durationMs * 0.5f));

    // Frame duration at 60 FPS ≈ 16.67 ms; round up to nearest frame
    float frameDurationMs = 1000.f / 60.f; // ≈16.67
    m_targetFrames = std::max(1, static_cast<int>(
        std::ceil(static_cast<float>(durationMs) / frameDurationMs)));

    log::info("[Blinky] Flash: {}ms → {} frames", durationMs, m_targetFrames);
    return true;
}

void FlashController::start() {
    m_frameCount = 0;
    m_running    = true;
    if (m_fragment) m_fragment->setVisible(true);
    scheduleUpdate();
}

void FlashController::update(float /*dt*/) {
    if (!m_running) return;
    m_frameCount++;
    if (m_frameCount >= m_targetFrames) {
        m_running = false;
        unscheduleUpdate();
        if (m_fragment) m_fragment->setVisible(false);
        if (m_callback) m_callback();
    }
}
