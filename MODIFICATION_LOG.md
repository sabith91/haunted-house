# Modification & Migration Log: Modularization of Haunted House Simulation

This document tracks all modifications, architectural splits, and file mappings made to decompose the original monolithic `haunted_house.cpp` (1,350 lines) into a clean, modular multi-file project.

If you ever encounter an issue or make an error in your code, you can use this document and the preserved backup file to inspect the original implementation or revert changes.

---

## 1. Safety & Rollback Backup

Before any modifications were performed, an exact, verbatim copy of the original monolithic file was saved as:
* **`haunted_house_backup.cpp`** (Identical to original 1,350-line file)

### How to Revert to the Single-File Version:
If you ever want to revert completely to the single-file setup:
1. Delete or rename the modular `.cpp` and `.h` files:
   - `primitives.h` / `primitives.cpp`
   - `train_railway.h` / `train_railway.cpp`
   - `house_exterior.h` / `house_exterior.cpp`
   - `house_interior.h` / `house_interior.cpp`
   - `grounds.h` / `grounds.cpp`
   - `dynamic_objects.h` / `dynamic_objects.cpp`
   - `lighting_sky.h` / `lighting_sky.cpp`
   - `common.h`
2. Restore the original code:
   ```powershell
   Copy-Item .\haunted_house_backup.cpp .\haunted_house.cpp
   ```
3. Revert `Makefile` rule to compile only `haunted_house.cpp`:
   ```makefile
   $(TARGET): haunted_house.cpp
   	$(CXX) -O2 -o $@ $< $(LIBS)
   ```

---

## 2. Modular Architecture Overview

The project was divided into 8 focused modules with distinct responsibilities:

| Module | Files | Responsibility |
|---|---|---|
| **Common Header** | `common.h` | Shared includes (`GL/glut.h`, `GL/glu.h`), math constants (`PI`, `DEG`, `WH`, `F0`, `F1`), data structs (`Box`, `Door`, `Spot`), display list IDs (`LG_GROUND`..`LG_COUNT`), extern global variables, inline LCG random generator (`rnd`, `rndr`), and collision registrar declaration (`solid`). |
| **Primitives & Materials** | `primitives.h`<br>`primitives.cpp` | Geometric helpers (`box`, `cyl`, `cylCap`, `cone`, `sph`, `ell`, `prism`, `prismZ`, `quadUp`), face grid subdivider (`faceGrid`), `bigBox`, collision-registering `wallBox`, interior `lining`, OpenGL material presets (`matStone`, `matWood`, `matRust`, `matIron`), emissive candle `flame`, and multi-layer `window` rendering. |
| **Train & Railway** | `train_railway.h`<br>`train_railway.cpp` | Track geometry formulas (`trackX`, `trackYaw`), rail and gravel display list builder (`buildRail` / `LG_RAIL`), rotating wheel geometry (`wheel`), train position formula (`trainHeadZ`), and locomotive + carriage renderer (`drawTrain`). |
| **House Exterior** | `house_exterior.h`<br>`house_exterior.cpp` | Exterior display list builder (`buildHouseExt` / `LG_EXT`), outer stone walls with plinth band, slate roof, cross gables, roof dormers (`dormer`), chimneys, porch columns and steps, round stone tower, and leaning timber turret (`towerAndTurret`). |
| **House Interior** | `house_interior.h`<br>`house_interior.cpp` | Interior display list builder (`buildHouseInt` / `LG_INT`), hall side walls, door gaps, upstairs floor slabs, ceiling beams, alternating floor planks, peeling wallpaper linings, 14-step staircase with balustrades and newel posts, gallery rail, side tables, portraits (`portrait`), Bedroom A (child room with plank wall, bed, dresser, radiator, rug, hanging lamp), Bedroom B (draped room with dark bed, candle table, rags), Bedroom C (abandoned room with striped wallpaper, two iron beds, wardrobe, floor holes). |
| **Grounds & Environment** | `grounds.h`<br>`grounds.cpp` | Procedural tree generator (`genTrees`), ground grid (`buildGround` / `LG_GROUND`), cobblestone path (`buildPath` / `LG_PATH`), fence and gates (`buildFence` / `LG_FENCE`, `buildLeaf` / `LG_LEAF`), graveyard tombstones (`buildGrave` / `LG_GRAVE`), yard props (`buildProps` / `LG_PROPS` - wagon, pumpkins, lamp posts, tree shadow blobs), and trees display lists (`buildTrees` / `LG_TREES`, `LG_PINES`). |
| **Dynamic Entities & FX** | `dynamic_objects.h`<br>`dynamic_objects.cpp` | Dynamic / animated scene elements drawn every frame: interactive doors (`drawDoorLeaf`), swinging estate gate (`drawGate`), swinging chandelier (`drawChandelier`), candelabra flames (`drawCandelabra`), cloth physics drapes (`drape`), rocking chair (`rockingChair`), lamp post bulbs (`drawLampBulbs`), Bedroom B candle (`drawCandleB`), orbiting bats and circling crows (`winged`, `drawBatsCrows`), drifting ghost (`ghostPos`, `drawGhost`), translucent moonlight beams (`drawBeams`), and low drifting fog sheets (`drawFogSheets`). |
| **Sky, Lighting & HUD** | `lighting_sky.h`<br>`lighting_sky.cpp` | Starfield generator (`initStars`, `stars`), sky dome with lightning flash effect and moon halo (`drawSky`), 8 fixed-function OpenGL lights with proximity culling (`setupWorldLights`, `setPointLight`), bitmap text renderer (`text`), zone detection (`zoneName`), and 2D heads-up display (`drawHUD`). |
| **Main Engine & Controller** | `haunted_house.cpp` | Global state definitions, frame rendering callback (`display`), step and floor height solver (`stepsH`, `floorH`), circle-box collision engine (`collide`), viewpoint teleporter (`setView`), door/gate interaction (`interact`), simulation physics and key update (`update`), GLUT input callbacks (`keyDown`, `keyUp`, `specialDown`, `specialUp`, `mouseMove`, `reshape`, `idle`), headless test-shot engine (`--shot`), and `main()`. |

---

## 3. Original Monolithic Code Mapping Table

Use this table to find exactly where any function from the original monolithic file now lives:

| Original Lines (`haunted_house_backup.cpp`) | Function / Symbol | Target Destination |
|---|---|---|
| 35–40 | Constants (`PI`, `DEG`, `WH`, `F0`, `F1`) | `common.h` |
| 42–87 | Global variables, structs (`Box`, `Door`, `Spot`), `enum LG_*` | `common.h` (declared `extern`), defined in `haunted_house.cpp` |
| 66–69 | Random generator `rnd()`, `rndr()` | `common.h` (inline functions) |
| 71–75 | Collision struct `Box` and `solid()` | `common.h` (decl), `haunted_house.cpp` (impl) |
| 92–136 | Primitives `col`, `box`, `cyl`, `cylCap`, `cone`, `sph`, `ell`, `prism`, `prismZ`, `quadUp` | `primitives.h` / `primitives.cpp` |
| 139–175 | Subdivided boxes `faceGrid`, `cells`, `bigBox`, `wallBox`, `lining` | `primitives.h` / `primitives.cpp` |
| 177–186 | Materials `mat`, `matStone`, `matWood`, `matRust`, `matIron` | `primitives.h` / `primitives.cpp` |
| 189–190 | Railway math `trackX()`, `trackYaw()` | `train_railway.h` / `train_railway.cpp` |
| 192–198 | Candle flame `flame()` | `primitives.h` / `primitives.cpp` |
| 204–244 | Window rendering `windowFace()`, `window()`, `litHash()` | `primitives.h` / `primitives.cpp` |
| 249–310 | Trees & generator `branch()`, `deadTree()`, `pineTree()`, `inHouseZone()`, `genTrees()` | `grounds.h` / `grounds.cpp` |
| 315–338 | Ground grid list `buildGround()` | `grounds.h` / `grounds.cpp` |
| 339–349 | Path stones list `buildPath()` | `grounds.h` / `grounds.cpp` |
| 351–407 | Fence, pillars, gargoyles, wall `pillar()`, `gargoyle()`, `woodSection()`, `ironFront()`, `buildFence()` | `grounds.h` / `grounds.cpp` |
| 409–427 | Graveyard list `tombstone()`, `buildGrave()` | `grounds.h` / `grounds.cpp` |
| 428–467 | Wagon, pumpkins, lamps, shadows `buildProps()` | `grounds.h` / `grounds.cpp` |
| 468–480 | Trees lists `buildTrees()` | `grounds.h` / `grounds.cpp` |
| 483–498 | Railway list `buildRail()` | `train_railway.h` / `train_railway.cpp` |
| 501–511 | Gate leaf list `buildLeaf()` | `grounds.h` / `grounds.cpp` |
| 516–546 | Tower and turret `towerAndTurret()` | `house_exterior.h` / `house_exterior.cpp` |
| 548–557 | Roof dormer `dormer()` | `house_exterior.h` / `house_exterior.cpp` |
| 559–618 | Mansion exterior list `buildHouseExt()` | `house_exterior.h` / `house_exterior.cpp` |
| 623–676 | Interior decor `portrait()`, `bed()`, `ironBed()`, `dresser()`, `radiator()`, `candelabraBase()` | `house_interior.h` / `house_interior.cpp` |
| 678–792 | Mansion interior list `buildHouseInt()` | `house_interior.h` / `house_interior.cpp` |
| 797–804 | Animated door leaf `drawDoorLeaf()` | `dynamic_objects.h` / `dynamic_objects.cpp` |
| 805–809 | Animated gate `drawGate()` | `dynamic_objects.h` / `dynamic_objects.cpp` |
| 810–823 | Swinging chandelier `drawChandelier()` | `dynamic_objects.h` / `dynamic_objects.cpp` |
| 824–827 | Candelabra with flames `drawCandelabra()` | `dynamic_objects.h` / `dynamic_objects.cpp` |
| 828–837 | Cloth drapes `drape()` | `dynamic_objects.h` / `dynamic_objects.cpp` |
| 838–848 | Rocking chair `rockingChair()` | `dynamic_objects.h` / `dynamic_objects.cpp` |
| 849–858 | Bulbs & candle B `drawLampBulbs()`, `drawCandleB()` | `dynamic_objects.h` / `dynamic_objects.cpp` |
| 861–905 | Train geometry & drawing `wheel()`, `trainHeadZ()`, `drawTrain()` | `train_railway.h` / `train_railway.cpp` |
| 908–930 | Bats and crows `winged()`, `drawBatsCrows()` | `dynamic_objects.h` / `dynamic_objects.cpp` |
| 931–944 | Ghost movement & drawing `ghostPos()`, `drawGhost()` | `dynamic_objects.h` / `dynamic_objects.cpp` |
| 945–957 | Moonlight beams `drawBeams()` | `dynamic_objects.h` / `dynamic_objects.cpp` |
| 958–966 | Drifting fog sheets `drawFogSheets()` | `dynamic_objects.h` / `dynamic_objects.cpp` |
| 971–1011 | Sky dome, stars, moon `stars`, `initStars()`, `drawSky()` | `lighting_sky.h` / `lighting_sky.cpp` |
| 1016–1057 | Fixed-function lights `setPointLight()`, `closeTo()`, `setupWorldLights()` | `lighting_sky.h` / `lighting_sky.cpp` |
| 1062–1086 | HUD `text()`, `zoneName()`, `drawHUD()` | `lighting_sky.h` / `lighting_sky.cpp` |
| 1091–1139 | Frame rendering `display()` | `haunted_house.cpp` |
| 1144–1247 | Physics & update loop `stepsH()`, `floorH()`, `collide()`, `setView()`, `initDoors()`, `interact()`, `update()` | `haunted_house.cpp` |
| 1252–1293 | GLUT input & windowing `keyDown()`, `keyUp()`, `specialDown()`, `specialUp()`, `mouseMove()`, `reshape()`, `idle()` | `haunted_house.cpp` |
| 1295–1313 | Initialization `init()` | `haunted_house.cpp` |
| 1316–1326 | Headless test shot `shotIdle()` | `haunted_house.cpp` |
| 1328–1350 | Main function `main()` | `haunted_house.cpp` |

---

## 4. Build System Updates

The build system was updated to compile each translation unit individually and link the final executable:

### Windows Build:
```powershell
# Using make.bat (calls mingw32-make)
.\make.bat

# Or direct invocation
mingw32-make
```

### Manual G++ Command (if make is not used):
```powershell
g++ -O2 -Wall -c primitives.cpp -o primitives.o
g++ -O2 -Wall -c train_railway.cpp -o train_railway.o
g++ -O2 -Wall -c house_exterior.cpp -o house_exterior.o
g++ -O2 -Wall -c house_interior.cpp -o house_interior.o
g++ -O2 -Wall -c grounds.cpp -o grounds.o
g++ -O2 -Wall -c dynamic_objects.cpp -o dynamic_objects.o
g++ -O2 -Wall -c lighting_sky.cpp -o lighting_sky.o
g++ -O2 -Wall -c haunted_house.cpp -o haunted_house.o
g++ -O2 -o haunted_house.exe *.o -lfreeglut -lglu32 -lopengl32 -static-libgcc -static-libstdc++
```
