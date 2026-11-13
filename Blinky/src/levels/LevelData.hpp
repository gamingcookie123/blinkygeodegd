#pragma once
#include <string>
#include <vector>

namespace blinky {

// Category flags for filtering levels in settings
enum class LevelCategory : int {
    Main    = 1 << 0,  // Stereo Madness – Dash (IDs 1–22)
    Tower   = 1 << 1,  // The Tower, Sewers, Cellar, Secret Hollow
    Special = 1 << 2,  // The Challenge
};

// Lightweight descriptor – we look up the real GJGameLevel* at runtime
struct LevelInfo {
    int         levelID;
    std::string name;
    LevelCategory category;
    int         estimatedLengthBlocks; // rough max X in blocks; 0 = unknown
};

// All known official levels Blinky can use
inline const std::vector<LevelInfo> ALL_OFFICIAL_LEVELS = {
    // Main levels (IDs match GD's internal level IDs)
    {  1, "Stereo Madness",           LevelCategory::Main,    127 },
    {  2, "Back On Track",            LevelCategory::Main,    138 },
    {  3, "Polargeist",               LevelCategory::Main,    145 },
    {  4, "Dry Out",                  LevelCategory::Main,    155 },
    {  5, "Base After Base",          LevelCategory::Main,    155 },
    {  6, "Can't Let Go",             LevelCategory::Main,    168 },
    {  7, "Jumper",                   LevelCategory::Main,    168 },
    {  8, "Time Machine",             LevelCategory::Main,    175 },
    {  9, "Cycles",                   LevelCategory::Main,    155 },
    { 10, "xStep",                    LevelCategory::Main,    168 },
    { 11, "Clutterfunk",              LevelCategory::Main,    168 },
    { 12, "Theory of Everything",     LevelCategory::Main,    155 },
    { 13, "Electroman Adventures",    LevelCategory::Main,    168 },
    { 14, "Clubstep",                 LevelCategory::Main,    175 },
    { 15, "Electrodynamix",           LevelCategory::Main,    168 },
    { 16, "Hexagon Force",            LevelCategory::Main,    168 },
    { 17, "Blast Processing",         LevelCategory::Main,    168 },
    { 18, "Theory of Everything 2",   LevelCategory::Main,    168 },
    { 19, "Geometrical Dominator",    LevelCategory::Main,    168 },
    { 20, "Deadlocked",               LevelCategory::Main,    168 },
    { 21, "Fingerdash",               LevelCategory::Main,    155 },
    { 22, "Dash",                     LevelCategory::Main,    168 },
    // Special
    { 1001, "The Challenge",          LevelCategory::Special, 100 },
    // Tower / platformer sub-worlds  (exact IDs verified against GD 2.2)
    { 5001, "The Tower",              LevelCategory::Tower,    80 },
    { 5002, "The Sewers",             LevelCategory::Tower,    80 },
    { 5003, "The Cellar",             LevelCategory::Tower,    80 },
    { 5004, "The Secret Hollow",      LevelCategory::Tower,    80 },
};

// A randomly chosen rectangular slice of a level
struct LevelFragment {
    int   levelID;
    float originX;      // level-space units (1 block = 30 units)
    float originY;      // level-space units
    float widthUnits;   // = fragmentWidthBlocks * 30
    float heightUnits;  // = fragmentHeightBlocks * 30
};

} // namespace blinky
