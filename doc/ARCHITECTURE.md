# Augustus Architecture Reference

This document is the fastest high-level reference for future Augustus development. It points to the subsystems that matter most, the files that anchor them, and the runtime flow that ties them together.

## 1. What the project is

Augustus is a fork of Julius focused on extending Caesar 3 with gameplay changes while keeping the original asset pipeline and broad platform support.

Core characteristics:

- C + CMake codebase
- SDL-based cross-platform runtime
- Data-heavy simulation updated on fixed game ticks
- Window-stack UI architecture
- Scenario/editor support built into the main runtime
- Save/load compatibility for importing Caesar 3 and Julius data, but Augustus saves are Augustus-specific

## 2. Source tree: mental model

Most important source directories under `src/`:

| Directory | Responsibility | Good entry files |
| --- | --- | --- |
| `platform/` | process entry, SDL integration, renderer bridge, file/prefs abstractions, platform-specific code | `src/platform/augustus.c`, `src/platform/renderer.c` |
| `game/` | startup, main runtime coordination, game state transitions, save/load orchestration, tick scheduling | `src/game/game.c`, `src/game/tick.c`, `src/game/file.c` |
| `city/` | city-wide simulation state and systems | `src/city/data.h`, `src/city/*.c` |
| `building/` | building definitions, behavior, production, services, house evolution | `src/building/building.h`, `src/building/properties.h` |
| `figure/` + `figuretype/` | moving entities, walkers, formations, figure AI | `src/figure/figure.h`, `src/figure/action.c` |
| `map/` | grid, terrain, desirability, water, routing, road networks | `src/map/grid.h`, `src/map/tiles.c`, `src/map/routing_terrain.c` |
| `scenario/` | scenario data, criteria, events, invasions, requests, custom event logic | `src/scenario/scenario.h`, `src/scenario/event/` |
| `editor/` | editor mode state and tools | `src/editor/editor.h`, `src/editor/tool.h` |
| `window/` | concrete screens/dialogs | `src/window/`, `src/window/editor/` |
| `widget/` | reusable UI widgets | `src/widget/` |
| `graphics/` | drawing primitives, fonts, text, window manager bridge, video | `src/graphics/window.c`, `src/graphics/text.c` |
| `assets/` | Augustus extra-asset loading from XML + PNG | `src/assets/assets.c` |
| `sound/` | audio init, music, city sounds, effects | `src/sound/system.c`, `src/sound/music.c` |
| `core/` | utilities: config, lang, buffers, XML, PNG, zip, strings, random, dir/file helpers | `src/core/` |
| `empire/` | world map and trade-partner state | `src/empire/` |
| `input/` | mouse, keyboard, touch, joystick, hotkeys | `src/input/` |

## 3. Runtime flow

### Startup

The main entry point is `src/platform/augustus.c`.

High-level flow:

1. `main()`
2. `platform_parse_arguments()`
3. `setup()`
4. `game_pre_init()` from `src/game/game.c`
5. `game_init()` or asset previewer
6. first `run_and_draw()`
7. main event loop

Important initialization responsibilities:

- SDL and windowing setup in `platform/`
- config, settings, hotkeys, language loading in `game_pre_init()`
- image/font/climate loading in `game_init()`
- building model reset and property initialization before gameplay
- sound system initialization
- initial window dispatch through `window_logo_show()`

### Per-frame loop

The runtime alternates between event processing and simulation/rendering:

- event loop in `main_loop()` (`src/platform/augustus.c`)
- simulation in `game_run()` (`src/game/game.c`)
- rendering in `game_draw()` (`src/game/game.c`)

`game_run()`:

- updates animations
- computes elapsed ticks from current game speed
- runs `game_tick_run()` one or more times per frame
- writes mission autosave state when needed

`game_draw()`:

- draws current window hierarchy through `window_draw(0)`
- plays city sound state through `sound_city_play()`

## 4. Simulation model

The most important scheduling file is `src/game/tick.c`.

### Tick cadence

`game_tick_run()` is the hub for simulation progression:

- editor mode: only minimal random/figure updates
- normal mode:
  - `random_generate_next()`
  - `game_undo_reduce_time_available()`
  - `advance_tick()`
  - `figure_action_handle()`
  - scenario special processors
  - victory checks

### Temporal layering

Simulation work is layered by time unit:

- **tick-level**: granular gameplay systems, routing, production, services, desirability
- **day-level**: sentiment, autosave, scenario events
- **month-level**: finance, health, invasions, requests, prices, trade and city progression
- **year-level**: ratings, yearly finance/trade resets, major long-horizon updates

When changing core gameplay, `src/game/tick.c` is usually the first place to inspect.

## 5. State ownership

Augustus is organized around subsystem-owned global/module state rather than a single central object graph passed through every call.

Key state domains:

- **city state**: `src/city/data.h`
- **scenario state**: `src/scenario/data.h` and related scenario modules
- **building state**: `src/building/building.h`
- **figure state**: `src/figure/figure.h`
- **map/grid state**: `src/map/*`
- **game/session state**: `src/game/state.h`, `src/game/settings.h`

Practical implication:

- feature work is usually localized to one subsystem
- cross-cutting changes often require touching both the scheduling layer (`game/tick.c`) and the owning subsystem
- save/load compatibility matters whenever state layout or serialized behavior changes

## 6. Save/load and scenario bootstrapping

Main entry file: `src/game/file.c`.

### What `file.c` does

- clears current scenario state
- rebuilds map and routing state
- initializes city/scenario/empire subsystems
- loads custom scenarios and saved games
- applies backward-compatibility fixes

Useful mental split:

- **scenario loading** = build a fresh city from scenario metadata
- **save loading** = restore a serialized in-progress city
- **editor loading** = special flow handled through `game/file_editor.*`

If a gameplay change affects persistent state, review:

- `src/game/file.c`
- `src/game/file_io.*`
- subsystem save/load helpers

## 7. Rendering, UI, and input

### Window system

The UI stack is managed in `src/graphics/window.c`.

Key properties:

- ring-buffer style window queue with `MAX_QUEUE = 5`
- each window provides callbacks for:
  - background draw
  - foreground draw
  - input handling
  - return handling
- redraw is invalidation-based (`window_invalidate()`, `window_request_refresh()`)

Mental model:

- `window/` = full screens and dialogs
- `widget/` = reusable controls inside windows
- `graphics/` = low-level drawing helpers

### Renderer

The platform/render bridge lives in `src/platform/renderer.c`.

Responsibilities include:

- texture/image atlas management
- backend abstraction for drawing
- unpacked image loading
- presenting the final frame

### Input

Input is normalized before it reaches windows:

- mouse/touch/joystick/keyboard state collection
- hotkey processing
- per-window input dispatch via `handle_input`

This routing is centered in `window_draw()` in `src/graphics/window.c`.

## 8. Assets and resources

Main file: `src/assets/assets.c`.

Important distinction:

- **base game assets** come from Caesar 3/Julius-compatible resources
- **Augustus extra assets** are loaded from XML-described asset lists and image groups

`assets_init()`:

- scans asset XML files
- builds group/image registries
- loads extra atlases
- exposes lookup helpers by logical names

This subsystem is important whenever adding:

- new UI art
- extra overlays
- new visual building/gameplay affordances

## 9. Editor and scenario tooling

Editor mode is not a separate app; it is another mode inside the same runtime.

Key files:

- `src/game/game.c` for mode transitions
- `src/editor/editor.*` for editor state
- `src/window/editor/` for editor UI
- `src/scenario/event/` for scenario event logic

This is a major architectural advantage:

- gameplay and editor share the same data structures
- scenario authoring can manipulate real runtime models
- editor features must still respect save/load, map integrity, and UI flow

## 10. Build and platform architecture

- top-level build definition: `CMakeLists.txt`
- developer docs location: `doc/`
- CI matrix: `.github/workflows/main.yml`

The project is designed to ship on many targets:

- Linux
- Windows
- macOS
- iOS
- Android
- Emscripten/Web
- Nintendo Switch
- PS Vita

Platform-specific adaptations live under `src/platform/<target>/`, while the common game logic stays in shared modules.

Rule of thumb:

- gameplay changes should stay out of platform directories
- platform changes should avoid leaking platform-specific assumptions into core simulation code

## 11. Highest-value entry points by task

| Task | Start here |
| --- | --- |
| Understand startup | `src/platform/augustus.c`, `src/game/game.c` |
| Change tick timing or simulation order | `src/game/tick.c` |
| Add/adjust building behavior | `src/building/`, `src/city/`, `src/game/tick.c` |
| Change house evolution/services | `src/building/house_*`, `src/city/` |
| Adjust walkers/AI | `src/figure/`, `src/figuretype/` |
| Change map/routing/water/desirability | `src/map/` |
| Add scenario logic or scripted behavior | `src/scenario/`, `src/scenario/event/` |
| Extend editor tooling | `src/editor/`, `src/window/editor/` |
| Add UI/window flow | `src/window/`, `src/widget/`, `src/graphics/window.c` |
| Add art/assets | `src/assets/`, `res/`, asset XML files |
| Debug save/load issues | `src/game/file.c`, `src/game/file_io.*` |
| Investigate audio issues | `src/sound/` |
| Investigate platform-specific bugs | `src/platform/` + target subdirectory |

## 12. Development guardrails

When making future changes, keep these architectural truths in mind:

1. **Tick order is gameplay.** Reordering systems in `game/tick.c` can change behavior even if each subsystem is untouched.
2. **State shape affects persistence.** Changes to state-owning modules often require save/load review.
3. **The window stack is the UI contract.** New screens should fit the existing invalidate/draw/input lifecycle.
4. **Editor mode is first-class.** New gameplay data often needs editor support and scenario serialization support.
5. **Assets are partly externalized.** New visuals may require both code and XML/image registration.
6. **Cross-platform support is part of the architecture.** Avoid solutions that only work on one desktop target unless the change is intentionally platform-specific.

## 13. Recommended reading order for a new contributor

1. `README.md`
2. `doc/BUILDING.md`
3. `doc/ARCHITECTURE.md`
4. `src/platform/augustus.c`
5. `src/game/game.c`
6. `src/game/tick.c`
7. one subsystem relevant to your task (`building/`, `city/`, `map/`, `scenario/`, or `window/`)
