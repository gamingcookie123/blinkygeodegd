#pragma once
#include <Geode/Geode.hpp>
#include "../levels/LevelData.hpp"

namespace blinky {

class LevelSelectionLayer : public cocos2d::CCLayer {
public:
    static LevelSelectionLayer* create(const LevelFragment& fragment);
    bool init(const LevelFragment& fragment);

private:
    void buildLevelList();
    void onLevelSelected(cocos2d::CCObject* sender);
    void filterList(const std::string& query);
    void goToPreview();

    LevelFragment m_fragment;
    int           m_selectedLevelID = -1;
    cocos2d::CCArray* m_allButtons = nullptr;
    cocos2d::extension::CCScrollView* m_scrollView = nullptr;
};

} // namespace blinky
