# Blinky

A **GeoGuessr-style minigame** for Geometry Dash.

> See a random fragment of an official level for a brief flash.  
> Identify the level.  Explore its full layout.  Guess the exact location.  
> Receive a penalty in blocks.  **Lower is better.**

---

## Gameplay

1. **BLINK** – A random rectangular fragment of an official RobTop level is shown for a brief moment (default ~50 ms, configurable).  
2. **IDENTIFY** – Select which level you think the fragment came from.  
3. **EXPLORE** – Scroll through the full selected level using drag/swipe.  
4. **GUESS** – Tap the location you think the fragment was taken from.  
5. **SCORE** – Receive penalty points equal to the Euclidean distance (in blocks) between your guess and the actual location.  A perfect guess = 0 points.

---

## Scoring

```
Penalty = round( Euclidean_distance_in_level_units / 30 )
```

One GD block = 30 level coordinate units.

| Distance | Penalty |
|----------|---------|
| Exact    | 0       |
| 1 block  | 1       |
| 10 blocks| 10      |
| 50 blocks| 50      |

---

## Game Modes

| Mode      | Rounds |
|-----------|--------|
| 5 Rounds  | 5      |
| 10 Rounds | 10     |
| Endless   | ∞      |

---

## Installation

### Requirements

- Geometry Dash **2.2081** (Windows)
- [Geode](https://geode-sdk.org/) installed

### From `.geode` file

1. Download `blinky.geode` from the Releases page.
2. Place it in your Geode mods folder:  
   `%localappdata%\GeometryDash\geode\mods\`
3. Launch Geometry Dash.
4. The **BLINK MODE** button will appear on the main menu.

---

## Building from source

### Prerequisites

- CMake ≥ 3.21  
- Visual Studio 2022 (MSVC) or Clang/LLVM  
- Geode SDK installed and `GEODE_SDK` environment variable set  
  ```
  set GEODE_SDK=C:\path\to\geode-sdk
  ```

### Build steps

```bash
git clone https://github.com/joel/blinky
cd blinky
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The compiled `blinky.geode` will be in `build/`.

---

## Settings

| Setting | Default | Description |
|---------|---------|-------------|
| Flash Duration (ms) | 50 | How long the fragment is visible. Minimum ≈ 17 ms (1 frame @ 60 FPS). |
| Fragment Width (blocks) | 16 | Width of the shown region. |
| Fragment Height (blocks) | 10 | Height of the shown region. |
| Difficulty | Normal | Easy = 3× longer flash. Hard = 0.5× shorter flash. |
| Sound Effects | On | Enable/disable sound. |
| Include Main Levels | On | Use the 22 main RobTop levels. |
| Include Tower Levels | On | Use tower/secret platformer levels. |
| Wrong Level Penalty | Off | +50 blocks if wrong level selected. |

---

## Supported Levels

### Main Levels (1–22)
Stereo Madness, Back On Track, Polargeist, Dry Out, Base After Base, Can't Let Go, Jumper, Time Machine, Cycles, xStep, Clutterfunk, Theory of Everything, Electroman Adventures, Clubstep, Electrodynamix, Hexagon Force, Blast Processing, Theory of Everything 2, Geometrical Dominator, Deadlocked, Fingerdash, Dash.

### Special
The Challenge (1001)

### Tower / Platformer
The Tower, The Sewers, The Cellar, The Secret Hollow

---

## Technical Notes

### Flash timing
Cocos2d-x is frame-based. At 60 FPS, the minimum reliable flash duration is **one frame ≈ 16.67 ms**. The configured `flash-duration-ms` is rounded up to the nearest frame boundary. At 50 ms, the fragment is shown for 3 frames.

### Coordinate system
- 1 block = 30 level coordinate units  
- Penalty = `round(sqrt(dx² + dy²) / 30)`  
- Target = centre of the original fragment  

### Level rendering
Objects are rendered using Cocos2d sprite frames from the GD game sheets. Unknown object IDs fall back to a plain coloured square. Trigger objects are hidden.

---

## Feature Status

| Feature | Status |
|---------|--------|
| Main menu button | ✅ Implemented |
| Blinky menu | ✅ Implemented |
| Mode selection (5/10/Endless) | ✅ Implemented |
| Settings (Geode native) | ✅ Implemented |
| Level database (22 main + specials) | ✅ Implemented |
| Level loading (real GJGameLevel*) | ✅ Implemented |
| Level string parsing | ✅ Implemented |
| Fragment generation with validation | ✅ Implemented |
| Flash system (frame-accurate) | ✅ Implemented |
| Difficulty multiplier | ✅ Implemented |
| Level selection UI | ✅ Implemented |
| Level preview (scrollable) | ✅ Implemented |
| Drag-to-scroll | ✅ Implemented |
| Tap-to-guess with confirm | ✅ Implemented |
| Coordinate conversion | ✅ Implemented |
| Scoring (Euclidean, block-based) | ✅ Implemented |
| Wrong level penalty option | ✅ Implemented |
| Round results screen | ✅ Implemented |
| End game / stats screen | ✅ Implemented |
| Play again / return to menu | ✅ Implemented |
| Level parse caching | ✅ Implemented |
| Error handling (missing levels) | ✅ Implemented |
| Tower level access | ⚠️ Best-effort (depends on local save) |
| Trigger filtering | ✅ Implemented |
| Color channel resolution | ✅ Implemented |
| Search in level selection | 🔲 Not yet implemented |
| Zoom in preview | 🔲 Not yet implemented |
| Sound effects | 🔲 Placeholder (setting present) |

---

## Mod ID

`joel.blinky`

## License

MIT
