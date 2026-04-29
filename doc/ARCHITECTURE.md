# Augustus Architecture Reference

This document is the long-lived developer memory for Augustus. It is meant to answer two questions fast:

1. **Where does a system live?**
2. **What is the safest entry point for changing it?**

## 1. Architectural summary

Augustus is a **portable C99 game engine** built as a **single-process, single-threaded, modular monolith**.

- **Platform shell**: SDL-driven startup, windowing, renderer, input, filesystem abstraction in `src/platform/`
- **Simulation core**: tick-based game logic in `src/game/`, `src/city/`, `src/building/`, `src/figure/`, `src/map/`
- **Presentation/UI**: rendering and modal windows in `src/graphics/`, `src/widget/`, `src/window/`
- **Content systems**: scenarios, editor, assets, translation, sound in `src/scenario/`, `src/editor/`, `src/assets/`, `src/translation/`, `src/sound/`
- **Build system**: top-level `CMakeLists.txt` plus CI scripts in `.ci_scripts/` and workflow matrix in `.github/workflows/main.yml`

The codebase favors **global state + ID-based lookup + fixed-size map grids** over object graphs and dynamic runtime composition.

## 2. What to read first

If you only read a few files, read these first:

| Purpose | File |
|---|---|
| App startup and SDL event loop | `src/platform/augustus.c` |
| High-level game init / runtime handoff | `src/game/game.c` |
| Main simulation tick | `src/game/tick.c` |
| Scenario/save bootstrap | `src/game/file.c` |
| Central city state | `src/city/data_private.h` |
| Grid model and fixed map size | `src/map/grid.h` |
| Window stack and modal UI flow | `src/graphics/window.c` |
| Extra/custom asset loading | `src/assets/assets.c` |
| Scenario event engine | `src/scenario/event/controller.c` |

## 3. Runtime flow

### Startup

The executable enters through `src/platform/augustus.c`.

High-level boot sequence:

1. Configure logging and SDL
2. Create screen/renderer abstractions
3. Call `game_pre_init()` in `src/game/game.c`
   - settings/config/hotkeys
   - base Caesar 3 language resources
   - locale/encoding/translation setup
   - PRNG initialization
4. Call either:
   - `game_init()` for normal gameplay
   - `game_init_editor()` for editor mode
5. Enter the SDL event loop and repeatedly run:
   - `game_run()`
   - `game_draw()`
   - `platform_renderer_render()`

### Frame loop vs simulation loop

The important split is:

- **Rendering is frame-driven**
- **Simulation is tick-driven**

`game_run()` in `src/game/game.c` advances animation every frame, then executes `game_tick_run()` once for each elapsed gameplay tick determined by game speed.

`game_tick_run()` in `src/game/tick.c` is the core simulation heartbeat:

- advances time
- updates city systems on scheduled sub-ticks
- updates figures
- processes scenario systems
- checks victory

This means most gameplay bugs should be traced from `src/game/tick.c`, not from the SDL loop.

## 4. Core architectural patterns

### 4.1 Global singleton state

The game uses large shared state containers instead of passing context objects through every subsystem.

The most important example is `city` in `src/city/data_private.h`:

- finance
- population
- labor
- migration
- sentiment
- health
- ratings
- emperor state
- military state
- figure/building counters

**Practical rule:** when you change gameplay behavior, first identify which part of `city` is the source of truth.

### 4.2 ID-based entity model

Buildings, figures, events, and many resources are referenced by integer IDs rather than raw pointers.

Why it matters:

- save/load is simpler
- sparse arrays can move without pointer invalidation
- cross-system references stay serialization-friendly

**Practical rule:** when adding relationships between systems, prefer storing IDs and resolving through the owning module.

### 4.3 Grid-centric world model

The map is built around fixed-size grid storage in `src/map/grid.h`.

- `GRID_SIZE` is hardcoded to `162`
- many subsystems store data as `GRID_SIZE * GRID_SIZE` arrays
- offsets are a first-class coordinate format

This is one of the strongest architectural constraints in the project.

**Practical rule:** if a feature depends on spatial state, expect it to touch one or more map grids instead of attaching data directly to a tile object.

### 4.4 Modal window stack

UI is managed by `src/graphics/window.c`.

- active windows are represented by `window_type`
- windows are pushed with `window_show()`
- previous windows are restored with `window_go_back()`
- the queue depth is intentionally small (`MAX_QUEUE` = 5)

Input is routed through the current window after shared preprocessing.

**Practical rule:** new UX flows usually mean adding a new window module under `src/window/` and optionally widgets under `src/widget/`.

### 4.5 Buffer-based serialization

Save/load is orchestrated from `src/game/file.c` and implemented in the file I/O layer under `src/game/file_io.c` plus subsystem-specific save helpers.

The architecture writes typed fields and grid data into buffers rather than dumping native structs wholesale.

Why it matters:

- versioning is manageable
- padding/alignment problems are avoided
- individual fields can evolve more safely

**Practical rule:** if you add persistent gameplay state, you must update both initialization and serialization paths.

## 5. Subsystem map

The top-level source layout in `src/` is a strong guide to ownership:

| Subsystem | Responsibility |
|---|---|
| `platform/` | SDL lifecycle, screen, renderer bridge, platform I/O, per-platform hooks |
| `core/` | buffers, config, file helpers, locale/encoding, utility infrastructure |
| `game/` | startup, speed, time, state, save/load entry points, tutorial, campaign |
| `city/` | city-wide economy, population, ratings, labor, religion, migration, finance |
| `building/` | building rules, maintenance, production, housing, monuments, menus |
| `figure/` | walkers, formations, traders, movement, path-driven actions |
| `figuretype/` | specialized figure behaviors separated by role/domain |
| `map/` | grid storage, terrain, routing, desirability, water, sprites, map properties |
| `empire/` | empire map, foreign cities, trade relationships, world-level data |
| `scenario/` | scenario properties, win/loss logic, invasions, requests, event controller |
| `editor/` | editor mode state shared by editor-facing windows/tools |
| `assets/` | custom XML + PNG asset loading on top of Caesar 3 resources |
| `graphics/` | renderer-facing drawing primitives, image/font/video/window services |
| `input/` | cursor, hotkeys, joystick, scrolling, touch-to-mouse integration |
| `widget/` | reusable UI pieces such as sidebars, minimap, top menu, city widgets |
| `window/` | full-screen screens/dialogs such as city, advisors, menus, editors |
| `sound/` | music and city sound playback |
| `translation/` | language-specific UI strings |

## 6. Data ownership and change routing

Use this as the fastest routing guide before editing:

| If you are changing... | Start here |
|---|---|
| Game startup, missing assets, intro flow | `src/game/game.c` |
| SDL events, fullscreen, resize, quit, logging | `src/platform/augustus.c` |
| Camera/window rendering loop | `src/graphics/window.c`, `src/window/city.c` |
| Monthly/yearly simulation behavior | `src/game/tick.c` |
| Economy, labor, finance, ratings | `src/city/` |
| Building placement/evolution/production | `src/building/`, `src/map/building*`, `src/window/city.c` |
| Figure spawning/movement/action logic | `src/figure/`, `src/figuretype/`, `src/map/routing*` |
| Pathfinding, terrain, overlays, water, desirability | `src/map/` |
| Save compatibility or new persistent fields | `src/game/file.c`, `src/game/file_io.c`, affected subsystem save state |
| Scenario triggers, conditions, formulas, actions | `src/scenario/event/` |
| Map editor behavior | `src/window/editor/`, `src/editor/`, `src/scenario/` |
| New art/UI asset definitions | `res/assets/` + `src/assets/` |
| Platform-specific filesystem/path behavior | `src/platform/file_manager.c`, per-platform directories |

## 7. Simulation anatomy

`src/game/tick.c` is the scheduling table for most city behavior.

Important characteristics:

- year/month/day/tick progression is centralized here
- many systems update on specific tick slots rather than every tick
- figures are updated every gameplay tick
- scenario progression is mixed into normal simulation, not isolated in a separate loop

Examples already visible in `game_tick_run()` and helpers:

- finance
- labor
- housing service decay
- trade
- festivals and games
- water supply updates
- road network updates
- desirability recalculation
- invasions and random events

**Practical rule:** before changing pacing or balance, check whether the system is tied to tick, day, month, or year transitions.

## 8. World, map, and entity interaction

The game world is not a scene graph. It is a combination of:

- fixed map grids
- figure arrays
- building arrays
- city/scenario singleton data

Typical interaction pattern:

1. world state is stored in grids and shared structs
2. arrays hold entity records
3. IDs connect entities to other entities and tiles
4. render code reads from map/entity state to draw the visible slice

This is why spatial features often require coordinated updates in:

- `src/map/`
- `src/building/`
- `src/figure/`
- `src/city/`

## 9. Save, scenario, and editor relationship

There are three closely related content paths:

### Save/load

`src/game/file.c` handles clearing, bootstrapping, and restoring scenario/city state.

This layer is responsible for:

- resetting previous scenario state
- rebuilding map grids
- initializing entry/exit points
- restoring time/state after load
- reconnecting scenario and empire data

### Scenario systems

`src/scenario/` owns:

- mission properties
- objective logic
- earthquake/invasion/request/price-change systems
- custom event logic

The event controller in `src/scenario/event/controller.c` is the extensibility seam for scripted scenario behavior.

### Editor mode

Editor mode reuses a lot of runtime code but swaps the active window and initialization path through `game_init_editor()`.

The important insight is that the editor is **not a separate application**. It is a different operating mode of the same engine.

## 10. Asset and presentation pipeline

The game combines:

- original Caesar 3 assets
- Augustus-specific XML/PNG assets from `res/assets/`

`src/assets/assets.c` is the central entry point for extra asset loading.

Notable characteristics:

- custom assets are discovered from XML definitions
- asset lookups are converted into internal image IDs
- runtime drawing usually consumes image IDs, not filesystem paths
- renderer/platform details stay below the gameplay layer

Presentation split:

- `graphics/` = low-level draw services and renderer integration
- `widget/` = reusable parts
- `window/` = full screens and dialogs

## 11. Platform and build architecture

### Build system

The source file inventory and platform switches are centralized in `CMakeLists.txt`.

Key points:

- module file groups are explicitly listed there
- `TARGET_PLATFORM` drives cross-platform configuration
- supported targets include desktop, Android, iOS, Switch, Vita, and Emscripten
- CI orchestration lives in `.github/workflows/main.yml`
- helper scripts for CI/local parity live in `.ci_scripts/`

### Platform seam

The intended split is:

- `src/platform/` knows SDL and host environment details
- gameplay subsystems should stay platform-agnostic

Important practical files:

- `src/platform/augustus.c`
- `src/platform/file_manager.c`
- `src/platform/renderer.c`
- `src/platform/screen.c`
- `src/platform/{android,ios,switch,vita,emscripten}/`

## 12. Hard constraints future work must respect

These are the most important constraints to keep in mind:

1. **Single-threaded design**  
   Simulation, rendering, and most state mutation assume one thread.

2. **Fixed grid size**  
   `GRID_SIZE` is hardcoded to `162`, and many systems implicitly depend on it.

3. **Global mutable state**  
   Behavior is easy to change but cross-system side effects are common.

4. **Serialization coupling**  
   New persistent state requires careful save/load updates.

5. **Modal UI stack**  
   UX changes should fit the existing window stack model unless deliberately redesigned.

6. **Platform portability requirement**  
   Changes in filesystem, renderer, input, or timing code must be reviewed across multiple target platforms.

## 13. Safe development heuristics

When adding or changing features:

- start from the owning subsystem instead of adding cross-cutting logic in random windows
- prefer extending existing state containers and ID flows over inventing pointer-heavy side structures
- verify whether a change needs:
  - initialization
  - per-tick updates
  - rendering
  - input wiring
  - save/load support
  - editor/scenario support
- keep platform code isolated to `src/platform/` where possible

## 14. Quick memory checklist for future sessions

Before implementing a feature, ask:

1. Which subsystem owns the source of truth?
2. Is this tick/day/month/year driven?
3. Does it need serialization?
4. Does it need editor/scenario support?
5. Does it need new assets or just new rules?
6. Is the change gameplay, UI, or platform?
7. Which one or two files are the real entry points?

If you can answer those seven questions, you usually know where to work next.
