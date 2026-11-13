#pragma once
#include "../levels/LevelData.hpp"
#include "../levels/LevelLoader.hpp"
#include <optional>

namespace blinky {

class FragmentGenerator {
public:
    static FragmentGenerator& get();

    // Pick a random level, then pick a random region with enough objects.
    // Returns false if no valid fragment could be found after retries.
    bool generate(LevelFragment& out);

private:
    FragmentGenerator() = default;

    // Count how many objects fall inside the proposed region
    int countObjectsInRegion(const ParsedLevel& parsed, const LevelFragment& frag);

    static constexpr int MIN_OBJECTS = 5;    // Reject fragments with fewer objects
    static constexpr int MAX_ATTEMPTS = 20;  // Per-level retry limit
};

} // namespace blinky
