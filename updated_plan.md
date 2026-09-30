# plan.md: Interactive 3D Haunted House Simulation

> **Purpose of this file.** This is a complete, self-contained specification. A developer or AI agent should be able to rebuild the whole project from this file alone, without the PDF report, screenshots or source code. Every coordinate, constant, formula and pitfall needed is written here. Follow sections 1 to 17 in order.

---

## 0. Quick facts

| Item | Value |
|---|---|
| Title | Interactive 3D Haunted House Simulation |
| Project type | Simple OpenGL university project, Roll 2107091, student MD.SABITH, CSE 3rd year |
| Language / API | C++ (C++11), OpenGL 1.x fixed-function pipeline, GLUT / FreeGLUT, GLU quadrics |
| Deliverable | Modular source files (`haunted_house.cpp`, `primitives.cpp`, `train_railway.cpp`, `house_exterior.cpp`, `house_interior.cpp`, `grounds.cpp`, `dynamic_objects.cpp`, `lighting_sky.cpp`), headers, `Makefile`, `make.bat`, `README.md`, and `MODIFICATION_LOG.md` |
| External assets | **None.** No model files, no textures, no audio. Everything is built from primitives |
| Mood | *Granny* (house horror) and *Alan Wake 2* (cold moonlit forest, fog, railway, flashlight) |
| Player | First-person, eye height 1.7, walks freely outside and inside |
| Target | 60 fps on ordinary lab PCs |

**Primitives allowed:** cube, prism (custom), cylinder, sphere, cone, torus, disk, quads, triangles, points, lines.

**Not allowed:** shaders, model loaders, external textures.

---

### 0.1 Modular Codebase Structure

The project is structured into focused translation units and header interfaces:

| Module | Source / Header | Responsibility |
|---|---|---|
| **Common Interface** | `common.h` | System headers, constants (`PI`, `DEG`, `WH`, `F0`, `F1`), structs (`Box`, `Door`, `Spot`), `LG_*` display list enum, extern global variables, inline LCG random functions (`rnd`, `rndr`), and collision registrar declaration (`solid`). |
| **Primitives & Materials** | `primitives.h`<br>`primitives.cpp` | Geometric helpers (`box`, `cyl`, `cone`, `sph`, `ell`, `prism`, `prismZ`, `quadUp`), face grid subdivider, `bigBox`, `wallBox`, `lining`, materials (`matStone`, `matWood`, `matRust`, `matIron`), `flame`, and `window`. |
| **Train & Railway** | `train_railway.h`<br>`train_railway.cpp` | Track curve geometry (`trackX`, `trackYaw`), rail display list builder (`buildRail`), wheel geometry (`wheel`), train position formula (`trainHeadZ`), and animated locomotive + carriages (`drawTrain`). |
| **House Exterior** | `house_exterior.h`<br>`house_exterior.cpp` | Outer walls, slate roof, cross gables, roof dormers, chimneys, porch columns/steps, round stone tower, and tilted timber turret (`towerAndTurret`, `dormer`, `buildHouseExt`). |
| **House Interior** | `house_interior.h`<br>`house_interior.cpp` | Hall staircase (14 steps), balustrades, gallery rail, wallpaper linings, side tables, candelabras, portraits, and Bedrooms A/B/C furniture (`buildHouseInt`). |
| **Grounds & Environment** | `grounds.h`<br>`grounds.cpp` | Estate terrain (`buildGround`), cobblestone path (`buildPath`), fence/gate leaf (`buildFence`, `buildLeaf`), graveyard tombstones (`buildGrave`), props/wagon (`buildProps`), and tree generators (`genTrees`, `buildTrees`). |
| **Dynamic Objects & FX** | `dynamic_objects.h`<br>`dynamic_objects.cpp` | Animated doors (`drawDoorLeaf`), gate (`drawGate`), swinging chandelier, flickering candles/lamps, cloth drapes, rocking chair, bats, crows, ghost (`drawGhost`), moonlight beams, and fog sheets. |
| **Sky, Lights & HUD** | `lighting_sky.h`<br>`lighting_sky.cpp` | Dome starfield (`initStars`, `drawSky`), 8 fixed-function OpenGL lights with proximity culling (`setupWorldLights`), 2D HUD text and status (`drawHUD`). |
| **Engine / Main Loop** | `haunted_house.cpp` | Global variables definitions, collision solver (`collide`), floor height resolver (`floorH`), viewpoints (`setView`), interaction logic, update physics, GLUT callbacks, and entry point `main()`. |

> **Audit & Rollback:** The complete modification history, line-by-line mapping, and rollback instructions are maintained in **`MODIFICATION_LOG.md`**. A pristine backup of the original single-file implementation is preserved as **`haunted_house_backup.cpp`**.

---

## 1. Build and run

```bash
# Windows (MSYS2 MinGW64 or mingw32-make)
.\make.bat
# or
mingw32-make
# or directly with g++:
g++ -O2 -Wall -c primitives.cpp -o primitives.o
g++ -O2 -Wall -c train_railway.cpp -o train_railway.o
g++ -O2 -Wall -c house_exterior.cpp -o house_exterior.o
g++ -O2 -Wall -c house_interior.cpp -o house_interior.o
g++ -O2 -Wall -c grounds.cpp -o grounds.o
g++ -O2 -Wall -c dynamic_objects.cpp -o dynamic_objects.o
g++ -O2 -Wall -c lighting_sky.cpp -o lighting_sky.o
g++ -O2 -Wall -c haunted_house.cpp -o haunted_house.o
g++ -O2 -Wall -o haunted_house.exe *.o -lfreeglut -lglu32 -lopengl32 -static-libgcc -static-libstdc++
.\haunted_house.exe

# Linux
sudo apt install g++ freeglut3-dev
make
./haunted_house

# macOS
make
./haunted_house
```

Includes: `<cstdio> <cstdlib> <cstring> <cmath> <vector>`, `<GL/glut.h>` (or `<GLUT/glut.h>` on macOS), `<GL/glu.h>`.

Avoid identifiers `near` and `far` (they are macros on Windows).

**Headless test mode (recommended for agents).** The program supports:

```
./haunted_house --shot <viewpoint 1-4> <out.ppm> [yawDeg pitchDeg [x z [feet]]]
```

It runs 90 simulation steps of 1/30 s, renders one frame, writes a PPM and exits. For viewpoint 4 it also sets `simTime = 16` so the train is near. To use it without a display on Linux:

```bash
sudo apt install xvfb libgl1-mesa-dri
Xvfb :99 -screen 0 1280x720x24 &
DISPLAY=:99 LIBGL_ALWAYS_SOFTWARE=1 ./haunted_house --shot 1 /tmp/v1.ppm
```

Convert PPM to PNG with Pillow (`Image.open(...).save(...)`) to inspect.

---

## 2. Controls

| Input | Action |
|---|---|
| W A S D | Walk forward / left / backward / right. 3 units/s, Shift = run at 6 units/s |
| Mouse | Look (cursor hidden and warped to window centre) |
| Arrow keys | Look (yaw 1.8 rad/s, pitch 1.2 rad/s) |
| F | Flashlight on/off |
| E | Open/close nearest door (within 3.2) or toggle the gate (within 6.5 of gate centre) |
| L | Trigger lightning |
| 1 / 2 / 3 / 4 | Teleport to gate / entrance hall / bedroom / railway |
| H | Show/hide help lines on HUD |
| M | Mouse look on/off (also shows/hides cursor) |
| Esc | Quit |

Use `glutIgnoreKeyRepeat(1)`. Track key-down and key-up for W A S D. Lowercase letters (`'A'..'Z'` to `'a'..'z'`) so Shift does not break movement. Read `glutGetModifiers() & GLUT_ACTIVE_SHIFT` in both key callbacks.

---

## 3. Conventions (read before writing any geometry)

### 3.1 Coordinate system
- **X** right, **Y** up, **Z** toward the viewer. 1 unit is about 1 metre.
- Player starts outside the gate at `(0, 0, 47)` looking toward `-Z` (yaw 0).
- Mansion is at the back (negative Z) so it is seen while walking up the path.

### 3.2 Yaw and movement
- `forward = (sin(yaw)*cos(pitch), sin(pitch), -cos(yaw)*cos(pitch))`. Yaw 0 looks along -Z.
- `right = (cos(yaw), 0, sin(yaw))`.
- Mouse: `yaw += dx*0.0025`, `pitch -= dy*0.0025`, pitch clamped to +-1.3. After each event, `glutWarpPointer(w/2,h/2)` and ignore the resulting synthetic event.

### 3.3 Rotation conventions (very easy to get wrong)
- `glRotatef(a, 0,1,0)` maps local +X to world `(cos a, 0, -sin a)`, and local +Z to `(sin a, 0, cos a)`.
- Window "outward" rotations: 0 = faces +Z, 90 = faces +X, 180 = faces -Z, -90 = faces -X.
- Vertical cylinder helper: `glRotatef(-90,1,0,0)` then `gluCylinder` (GLU cylinders extend along +Z, so this makes them point along +Y). Same for `glutSolidCone`.
- A box long along Z tilted with `glRotatef(theta,1,0,0)` raises its **-Z** end when theta is positive.

### 3.4 Helper library (write these first)

| Helper | Meaning |
|---|---|
| `col(r,g,b)` | `glColor3f` (lighting uses `GL_COLOR_MATERIAL`) |
| `box(cx,cy,cz,w,h,d)` | Centred cube, `glScalef` + `glutSolidCube(1)` |
| `cyl(r0,r1,h,slices)` | Vertical tapered cylinder from y=0 up. `cylCap` adds a top disk |
| `cone(r,h,slices)` | Vertical cone, base at y=0 |
| `sph(x,y,z,r,slices)`, `ell(x,y,z,rx,ry,rz,slices)` | Sphere / ellipsoid |
| `prism(cx,y0,cz,w,h,d)` | Triangular prism, ridge along X (length w), cross-section d wide and h high, base on y0. Custom quads and triangles with correct normals |
| `prismZ(cx,y0,cz,len,h,wid)` | Same but ridge along Z (rotate 90 about Y) |
| `quadUp(x0,z0,x1,z1,y)` | Upward-facing flat quad |
| `bigBox(x0,y0,z0,x1,y1,z1,r,g,b,jit,cell)` | Box with all six faces subdivided into cells of about `cell` units. Each cell gets colour multiplied by `1 + (rnd-0.5)*2*jit`. **Reason:** fixed-function lighting is per vertex, so big faces would never show the flashlight or point lights. Cells give a stone/plank look too |
| `wallBox(...)` | `solid(...)` collision registration + `bigBox` in one call |
| `lining(...)` | `bigBox` with jitter 0.14 and cell 0.8 for interior wallpaper, no collision |
| `mat(r,g,b,spec,shin)` | Colour + `GL_SPECULAR` + `GL_SHININESS`. Presets: stone (0.32,0.32,0.34, spec 0.03), wood (0.25,0.16,0.09, 0.05), rust (0.45,0.22,0.10, 0.10), iron (0.07,0.07,0.08, 0.35, shin 30) |

### 3.5 Deterministic randomness
Use a tiny LCG: `rs = rs*1664525u + 1013904223u; return ((rs>>8)&0xFFFF)/65535.0f`. **Reseed (`rs = N`) at the start of every builder** so the scene is identical on every run and every list is stable.

### 3.6 Emissive things
Lit windows, flames, bulbs, moon, headlight: draw with `glDisable(GL_LIGHTING)` then re-enable. Fog still applies (good).

---

## 4. Global state and data structures

```cpp
const float PI, DEG=57.2957795f;
const float WH = 9.0f;   // mansion wall height
const float F0 = 0.30f;  // ground-floor level (plinth)
const float F1 = 4.50f;  // upstairs floor (= F0 + 14 steps * 0.3)

struct Box  { float x0,y0,z0,x1,y1,z1; };            // collision AABB
std::vector<Box> solids;                              // static colliders
struct Door { float hx,hy,hz, baseRot, w,h, sgn,maxA; float open,target; bool latch; Box gap; };
Door doors[4];                                        // 0 front, 1..3 bedrooms
float gateOpen; bool gateToggle;
struct Spot { float x,z,s; unsigned seed; };
std::vector<Spot> deadTrees, pines;
enum { LG_GROUND, LG_PATH, LG_FENCE, LG_TREES, LG_PINES, LG_GRAVE, LG_EXT, LG_INT, LG_RAIL, LG_PROPS, LG_LEAF, LG_COUNT };
GLuint base = glGenLists(LG_COUNT);                   // list id = base + LG_x
```

Player: `px, pz, feet` (floor height under feet), `yaw, pitchA`, `walkPhase`. Camera Y = `feet + 1.7 + 0.035*sin(walkPhase)`.

Time and atmosphere: `simTime`, `fogDens` (starts 0.018), `indoorAmt` (0..1 eased), `flashV` (lightning 0/1), `flashT`, `nextFlash`, flicker values `fChand, fCandle, fLamp`.

Fixed values:
- Fog base colour `(0.08, 0.12, 0.13)`; also the clear colour.
- Window size 1280x720; perspective FOV 65, near 0.1, **far 400**.

---

## 5. World layout (all coordinates)

### 5.1 Overview

| Zone | Position | Contents |
|---|---|---|
| Compound | X,Z in [-40,40] | Fence, gate at Z=+40, 8 wide opening at X in [-4,4] |
| Path | X in [-1.5,1.5], Z from 60 down to -11.5 | Cobble quads, then 5 porch steps to the door |
| Mansion footprint | X in [-12.2,12.2], Z in [-30.2,-13.8] (walls 0.4 thick centred on X=+-12, Z=-14, Z=-30) | Hall, gallery, 3 bedrooms |
| Round tower | centre (-13.5, -11.8), radius 2.8, height 14 | Front-left corner |
| Timber turret | centre (9, -27) | Back-right |
| Graveyard | X in [-38,-22], Z in [12,34] | 6 rows x 4 columns of tombstones, ghost |
| Wagon | (13, 0, -6), rotated 25 degrees | Yard prop |
| Lamp posts | (-7, 38) and (7, 38) | Beside the gate, inside the fence |
| Railway | X = 55 + 8*sin(Z/25), Z in [-170,170] | Outside the east fence |
| Forest | Outside fence on all sides | Pines, thicker fog near railway |

Wall inner faces (important for placing interior items): front wall inner face z=-14.2, back wall inner face z=-29.8, west inner face x=-11.8, east inner face x=+11.8, hall side wall faces x=-5.8 and x=+5.8.

### 5.2 Height helper functions
```cpp
float trackX(float z)   { return 55 + 8*sinf(z/25); }
float trackYaw(float z) { return atan2f(8.0f/25.0f*cosf(z/25), 1) * DEG; }  // degrees about Y
```

### 5.3 Ground and floor levels
- Outdoor ground is y=0. The mansion interior floor is `F0=0.3`. Upstairs floor is `F1=4.5`.
- Porch steps (5 cubes, step k=1..5): centre `(0, 0.03k, -11.75 - 0.5(k-1))`, size `(4.4, 0.06k, 0.5)`. Heights 0.06 to 0.30, joining the plinth.

---

## 6. Static display lists (build once in `init`)

Each builder starts with `glNewList(base+LG_x, GL_COMPILE)`, reseeds `rs`, ends with `glEndList()`. Call order in `init`: `genTrees, initDoors, buildGround, buildPath, buildFence, buildTrees, buildGrave, buildProps, buildHouseExt, buildHouseInt, buildRail, buildLeaf`.

### 6.1 `LG_GROUND`
- Normal (0,1,0). Outer ring: 8x8 cells over [-144,144], **skip** cells fully inside [-48,48]; colour `(0.04,0.09,0.05)*k`, k random 0.85 to 1.15.
- Inner: 2x2 cells over [-48,48]; colour `(0.06,0.14,0.07)*k`, k random 0.8 to 1.2.
- 260 grass tufts: 3 thin cones each (`cone(0.035, 0.4..0.8, 4)`, random yaw and small tilt), colour `(0.07,0.17,0.08)`. Skip tufts on the path (`|x|<2.2 && z>-14`) and **inside the mansion** (`|x|<13 && -31<z<-13`), otherwise they poke through the floor.

### 6.2 `LG_PATH`
Grid of stones, 0.75 pitch, x from -1.5 to 1.5, z from -11.5 to 60. Each stone is a quad ~0.69 wide at y=0.02 (small gaps show the ground), colour `(0.30,0.30,0.32)*k`, k random 0.65 to 1.35.

### 6.3 `LG_FENCE`
- **Wooden sections** (posts every 4 units, about 80 total): back `(-40,-40)->(40,-40)`, east `(40,-40)->(40,40)`, west `(-40,-40)->(-40,12)` and `(-40,36)->(-40,40)`.
  - Post: `cyl(0.15,0.13,2,8)` + cone cap `cone(0.17,0.25,8)`. 25% tilt up to 6 degrees.
  - Rails: boxes 0.1 tall, 0.06 thick, at y=1.6 and y=0.7, 30% tilted up to 5 degrees.
- **Iron front** (Z=40): pickets from x=-40 to -4.95 and 4.95 to 40, spacing 0.25: `box(x,1.0,40, 0.06,2.0,0.06)` + `cone(0.05,0.18,4)` at y=2.0. Rails: `box` 0.08 x 0.08 at y=1.8 and y=0.5. Colour iron `(0.07,0.07,0.08)`.
- **Brick wall**: `bigBox(-40.15,0,12, -39.85,2.2,36)` colour `(0.30,0.13,0.10)`, jit 0.16, cell 0.5, plus a capping box at y=2.3.
- **Stone pillars** (`pillar(x,z)`: `box(x,1.6,z, 0.9,3.2,0.9)`, cap `box(x,3.3,z,1.15,0.2,1.15)`, base `box(x,0.25,z,1.1,0.5,1.1)`, sphere top r=0.3 at y=3.75) at: the four corners, `(+-4.5,40)`, `(+-22,40)`, `(0,-40)`, `(40,0)`, `(-40,0)`, `(-40,12)`, `(-40,36)`.
- **Gargoyles** on top of the two gate pillars at `(+-4.5, 3.4, 40)`: cube body 0.5x0.6x0.4, sphere head r=0.2, two cone horns, two flattened cones as wings (+-55 degrees). Colour `(0.18,0.18,0.20)`.

### 6.3b `LG_LEAF` (gate leaf in local space, hinge at origin, extends +X, width 4, height 2.6)
Frame: bottom rail y=0.35, top rail y=2.45, two stiles (0.1 wide); vertical cylinder bars `r=0.03` every 0.3 with cone tips; a rust torus ring ornament in the middle. Iron material.

### 6.4 `LG_TREES` and `genTrees()`
- 40 dead trees at pseudo-random positions in [-38,38]. Reject if: on the path (`|x|<3.5 && z>-16`), near the house (`inHouseZone` with margin 4), near the wagon (`|x-13|<4 && |z+6|<4`), near the gate (`|x|<10 && z>32`).
- Tree = tapered trunk `cyl(0.38,0.11,4.6,8)` + 4 to 6 branches at heights 1.8 to 4.4, tilted 30 to 60 degrees, each branch recursive (`branch(len,r,depth)` with depth 1, 2 to 3 sub-branches at 25 to 60 degrees). No leaves. Colour `(0.15,0.11,0.08)`.
- Each tree also registers a collision box (0.7 x 0.7).
- **Pines** (`LG_PINES`): trunk `cyl(0.28,0.18,3.2,6)` + three cones (r 2.3/1.8/1.2, h 3.6/3.2/3.0, 8 slices, at y=2.0/4.0/5.9), dark green `(0.03..0.04, 0.10..0.12, 0.06..0.07)`, random scale 0.9 to 1.6.
  - Rows on both sides of the track: for z from -150 to 150 step 5.5, two rows per side at offset 6.5..9.5 and 12..19 from `trackX(z)`.
  - Plus 230 random pines in [-125,125]^2, rejecting `|x|<47 && |z|<47`, `|x-trackX(z)|<22`, and the gate approach `|x|<8 && z>40`.

### 6.5 `LG_GRAVE`
6 rows x 4 columns starting at `(-36, 14.5)` with 3.8 / 3.6 spacing plus jitter. Types: rounded top (box + squashed sphere), flat top (box + slab), cross (two boxes). Random leaning (Z +-7 degrees, X +-5 degrees), stone grey `(0.32,0.32,0.34)*k`, base slab. Each registers a collision box.

### 6.6 `LG_PROPS`
- **Wagon** at (13,0,-6): bed box 3.2x0.12x1.5 at y=0.95, slanted side boards, end board, tongue, four wheels as `glutSolidTorus(0.06,0.5,8,18)` with four crossed spoke boxes (one wheel missing, one tilted 14 degrees). Collision box x 11.4..14.6, z -7.4..-4.6.
- **4 pumpkins**: squashed orange spheres `(0.85,0.38,0.05)` at (-4,-9), (5.5,-9.5), (-9,-8), (-33,15), green stems.
- **Lamp posts** at (+-7,38): rust base and pole `cyl(0.07,0.06,3.2)`, cone shade at y=3.25. (Bulbs are dynamic.)
- **Blob shadows**: dark quads (`(0.02,0.05,0.03)`, lighting off) 2x2 under every dead tree at y=0.015.

### 6.7 `LG_RAIL`
- For z from -170 to 168 in steps of 2: at `zm=z+1, xm=trackX(zm)`, translate and rotate by `trackYaw(zm)`. Gravel box `(0,0.05,0) 3.4x0.1x2.06` colour `(0.13,0.13,0.14)`, two rails `box(+-0.75,0.24,0, 0.1,0.16,2.05)` rust-grey `(0.24,0.17,0.13)` (gauge 1.5, rail top at y=0.32).
- Sleepers every 0.7 units: `box(0,0,0, 2.4,0.1,0.26)` at y=0.15, dark brown with random brightness.

---

## 7. Mansion exterior (`LG_EXT`)

Colours: stone `(0.32,0.32,0.34)`, jitter 0.10, cell 1.0. Wall thickness 0.4. All outer walls call `wallBox`.

### 7.1 Outer walls (y from 0 to WH=9)

| Wall | Box (x0,z0)-(x1,z1) |
|---|---|
| Front left | x -11.8..-1.2, z -14.2..-13.8 |
| Front right | x 1.2..11.8, z -14.2..-13.8 |
| Front over door | x -1.2..1.2, y 3.4..9 |
| Back | x -11.8..11.8, z -30.2..-29.8 |
| West | x -12.2..-11.8, z -30.2..-13.8 |
| East | x 11.8..12.2, z -30.2..-13.8 |

(Front and back stop at +-11.8 so the side walls own the corners; this avoids coplanar z-fighting.) A darker lower band (y 0..1.4, colour `(0.2,0.2,0.22)`, 0.1 thick) is added outside each wall except the door gap.

### 7.2 Roof and details
- Main roof: slate `(0.09,0.10,0.12)`, `prism(0, 8.9, -22, 25.6, 3.1, 17.8)`. Ridge at y=12.0.
- Two cross gables: `prismZ(+-7.5, 9.0, -16.4, 5.2, 2.7, 5.0)`. Each has an arched window in its front face (one lit, one dark).
- Four dormers at `(+-3,-18)` rot 0 and `(+-3,-26)` rot 180: box 1.4x1.4x1.2 centred y=10.9, prismZ roof, small window quad.
- Three chimneys (boxes 1.1 wide, base y=9.6, caps 1.45 wide): tops at y=15.5 at (-8,-23), y=14.0 at (9.5,-20), y=16.2 at (2.5,-27.5).
- **Porch:** two columns `cylCap(0.24,0.22,3.6)` at `(+-2.3,-11.5)`; lintel `box(0,3.85,-11.4, 5.3,0.5,0.6)`; a dark torus ring under the lintel; prismZ porch roof `prismZ(0,4.1,-12.4, 3.8,1.5,5.8)`; two side blocks; five steps (see 5.3). Collision boxes at the columns.
- **Round tower:** centre (-13.5,-11.8): `cyl(2.9,2.8,14,20)` colour `(0.30,0.30,0.32)`, a torus band at y=13.6, cone roof `cone(3.4,6,20)` at y=14 (slate), spire `cone(0.16,3,6)` at y=19.8. Arched slits: 3 heights x 6 angles, quads 0.44 x 1.1 plus a half-disc top, mostly dark, every 4th lit amber. Collision box +-2.7 around the centre, height 14.
- **Timber turret** centre (9,-27): block1 `box(9,11,-27, 3.0,4.0,3.0)` (y 9..13), block2 `box(9,14.75,-27, 2.6,3.5,2.6)` (y 13..16.5), thin dark bands between, top block `box(0,1.5,0, 2.3,3,2.3)` rotated **7 degrees about Z** at y=16.5 with a prism roof (both `prism` and `prismZ`), 3 small windows. Collision box x 7.4..10.6, z -28.6..-25.4.

### 7.3 Windows (emissive quads, `window(x,y,z,rot,w,h,outerType,innerType,arch)`)
Types: 0 dark and cracked (3 grey crack lines), 1 lit amber `(1.0,0.78,0.30)`, 2 pale moonlight `(0.30,0.44,0.50)`, 3 pale plus vertical iron bars every 0.2. All have a dark cross mullion. Arched windows add a half-disc fan on top.

Outer quad is 0.22 from the wall centre plane (just outside the 0.2 half-thickness); inner quad is **0.30** inside (0.1 in front of the inner face) so interior wallpaper never covers it. `innerType = -1` means no inner quad.

Lit choice: `litHash(a,b,c) = ((int)(a*3+b*7+c*5+100)%3+3)%3`; lit when hash==0 (about one third).

| Wall | Positions | Size | Inner |
|---|---|---|---|
| Front (rot 0, z=-14) | x=-8 and 8, y=2.4 and 6.6 | 1.0 x 1.6 | upper ones type 2 |
| Front hall arches | x=+-3.6, y=2.6 and 6.8 | 0.9 x 1.4 arch | type 2 |
| West (rot -90, x=-12) | z=-18,-22,-26; y=2.4 and 6.6 | 1.0 x 1.6 | upper type 3 (barred) |
| East (rot 90, x=12) | z=-18,-22,-27.5; y=2.4 and 6.6 | 1.0 x 1.6 | upper type 2 |
| Back (rot 180, z=-30) | x=-8, 8: y=2.4 (no inner), y=6.6 (type 2) | 1.0 x 1.6 | |
| Back hall arches | x=-3.6, 0, 3.6, y=6.8 | 0.9 x 1.4 arch | type 2 |

---

## 8. Mansion interior (`LG_INT`)

Hall colour (dark green-grey, R8 look): `(0.17,0.21,0.20)`, jitter 0.12.

### 8.1 Hall side walls with door gaps
Ground part y 0..F1 (4.5), full length, at x in [-6.2,-5.8] and [5.8,6.2], z in [-29.8,-14.2]. Upper part y F1..WH:

| Wall | Solid segments (z ranges) | Door gap (z) | Lintel |
|---|---|---|---|
| West x=-6 | -29.8..-24.6 and -23.4..-14.2 | -24.6..-23.4 (Door 1) | y 7.5..9 over the gap |
| East x=+6 | -29.8..-28.1, -26.9..-23.1, -21.9..-14.2 | -28.1..-26.9 (Door 2), -23.1..-21.9 (Door 3) | y 7.5..9 over each gap |

Partition between bedroom B and C: `wallBox(6.2,F1,-25.2, 11.8,WH,-24.8)` colour `(0.14,0.15,0.18)`.

### 8.2 Floors, ceiling
- Upstairs slabs (y 4.2..4.5, colour `(0.20,0.13,0.08)`, jit 0.22, cell 0.5): gallery x -5.8..5.8, z -29.8..-21.3; wing A x -11.8..-6.2, z -29.8..-14.2; wings B/C x 6.2..11.8, z -29.8..-14.2.
- Ceiling slab y 9..9.1 over the whole footprint. Ceiling beams (0.3x0.3x11.6) every 3 units from z=-16.5 to -28.5.
- Hall ground floor: a dark underlay quad at y=F0-0.02, then 24 plank columns (0.483 wide) x 8 segments (1.95 long) at y=F0; alternating brightness (0.75 / 1.05); 1 in 40 planks missing and some raised 0.06.
- Ground-floor wings are **sealed and empty** (not enterable).

### 8.3 Staircase (west half of the hall)
- 14 steps, index i=0..13: x from -5.8 to -2.0, z from `-15 - 0.45i` to `-15 - 0.45(i+1)`, solid from y=0 to `F0 + 0.3(i+1)`. Top step top = 4.5 at z=-21.3. Colour `(0.20,0.13,0.08)`, alternating 0.9/1.1 brightness.
- Balustrade on the x=-2.05 side: posts `cyl(0.03,0.03,0.9)` on each step, sloped handrail box 0.1 x 0.09 x length, centre `(-2.05, F0 + rise/2 + 1.0, -15 - run/2)` with `rise = 4.2, run = 6.3`, rotated `atan2(rise,run)` degrees about X (positive raises -Z end). Newel posts (box 0.2x1.3x0.2 + sphere r=0.14) at z=-15 and z=-21.3.
- Collider for the stair rail: x -2.15..-1.95, z -21.4..-14.9, y F0..F1+1.2.
- **No collision for the step blocks.** Walking on them is handled by `floorH` (see 12).

### 8.4 Gallery rail
Posts every 0.3 from x=-1.9 to 6.0 at z=-21.3 (height 0.9 above F1), handrail box 8.0 long at y=F1+0.95, newel post at x=6. Collider x -2.1..6.1, z -21.45..-21.15, y F1..F1+1.2.

### 8.5 Hall decor
- Side tables (1.2x0.8x0.5) at (5, F0+0.4, -15.2) and (5, F0+0.4, -27.5), with colliders. Candelabras stand on them and on the stair newel post at `(-2.05, F0+1.5, -15)` (dynamic flames).
- Portraits (frame box + dark quad + pale face oval + shoulders): west wall at `(-5.75, 2.9, -17.6)` and `(-5.75, 4.0, -20.3)` rot 90; east wall at `(5.75, 2.7, -16.6)` and `(5.75, 2.7, -19.6)` rot -90 (**this one tilted 7 degrees**); back wall at `(-2.2, 6.9, -29.72)` and `(2.2, 6.9, -29.72)`. Size 0.9x1.3 (1.1x1.5 on back wall).
- Back-wall lining `lining(-5.8,F0,-29.8, 5.8,WH,-29.76)` plus 9 random dark peeling-patch quads at z=-29.75.

### 8.6 Bedrooms (interior spaces)

| Room | Bounds | Door | Reference mood |
|---|---|---|---|
| A (child room) | x -11.8..-6.2, z -29.8..-14.2 | Door 1 at x=-6, z -24.6..-23.4 | R9 plank room |
| B (draped room) | x 6.2..11.8, z -29.8..-25.2 | Door 2 at x=6, z -28.1..-26.9 | R10 dark bed and candle |
| C (abandoned) | x 6.2..11.8, z -24.8..-14.2 | Door 3 at x=6, z -23.1..-21.9 | R11 iron beds, wardrobe |

**Lining direction pitfall:** a lining must sit **in the room**, in front of the wall's inner face. E.g. back wall inner face is z=-29.8, so the lining is z in [-29.8,-29.76], not [-29.84,-29.8] (that would be inside the wall and z-fight). Front inner face is z=-14.2, lining z in [-14.24,-14.2].

**Room A:** lining on back and front walls `(0.20,0.16,0.12)`; plank wall on the west face: 9 boxes 0.04 thick at x=-11.76, y from F1+0.25 step 0.5, each 0.46 tall, 15.6 long, brown with random brightness; bed at `(-9.5,F1,-28.7)` (1.6 x 2.2, head at the back wall); dresser at `(-11.35,F1,-24)` (0.9x1.0x1.6, three drawers, knobs, faces +X); radiator (9 thin fins) at `(-11.6,F1,-18)`; red rug quad and open book on floor; hanging lamp (chain + green cone shade + emissive bulb) at `(-9, WH-1.5, -22)`. Colliders: bed x -10.4..-8.6, z -29.8..-27.5; dresser x -11.8..-10.85, z -24.9..-23.1.

**Room B:** lining `(0.09,0.11,0.15)` on back, east and partition faces; large dark bed at `(10,F1,-28.7)` (2.0 x 2.2); bedside table `box(8.2,F1+0.3,-29.3, 0.8,0.6,0.7)` with a candle cylinder on it; 6 dark "rag" quads on the floor. Colliders for bed and table. Very low light.

**Room C:** lining `(0.24,0.22,0.18)` on front and partition faces; east wall striped wallpaper: boxes 0.04 thick at x=11.74, one per 0.5 in z from -24.6 to -14.3, alternating two beige tones; iron beds (thin cylinders, mattress box) at `(10.7,F1,-23.3)` and `(10.7,F1,-19.5)` rotated **-90** (head at east wall); wardrobe `box(11.4,F1+1.1,-15.9, 0.7,2.2,1.6)`; three dark hole quads in the floor at (8.5,-17.5), (9.2,-15.8), (7.2,-19.6); a grey dust quad. Colliders for both beds and wardrobe.

---

## 9. Dynamic objects (drawn every frame)

### 9.1 Doors and gate
Door table (built by `initDoors`):

| # | Hinge (x,y,z) | baseRot | Width | Height | sgn | maxA | Closed collision box (gap) |
|---|---|---|---|---|---|---|---|
| 0 Front | (-1.2, F0, -14.0) | 0 | 2.4 | 3.0 | +1 | 90 | x -1.2..1.2, y F0..3.4, z -14.15..-13.85 |
| 1 Bedroom A | (-6.0, F1, -23.4) | 90 | 1.2 | 3.0 | +1 | 90 | x -6.15..-5.85, y F1..7.5, z -24.6..-23.4 |
| 2 Bedroom B | (6.0, F1, -26.9) | 90 | 1.2 | 3.0 | -1 | 90 | x 5.85..6.15, z -28.1..-26.9 |
| 3 Bedroom C | (6.0, F1, -21.9) | 90 | 1.2 | 3.0 | -1 | 90 | x 5.85..6.15, z -23.1..-21.9 |

Draw: translate to hinge, `glRotatef(baseRot + sgn*open*maxA, 0,1,0)`, then a box `box(w/2, h/2, 0, w, h, 0.1)` (dark wood `(0.22,0.14,0.08)`) with two raised panels per side and two brass knobs. `open` is 0..1, eased at 0.9 per second toward `target`. A door is a collision box only while `open < 0.5`.

Behaviour:
- E: pick nearest door with distance to the gap centre < 3.2 **and** on the same floor (`|feet - (hy - floorLevel)| < 1.5`); toggle `target`.
- Door 1 auto-opens once (sets `latch`) when the player is upstairs (`feet>3`), `|px+5|<3` and `|pz+24|<3` ("creaks open").
- Gate: leaf 1 at hinge (-4,0,40): `glRotatef(gateOpen*70, 0,1,0)`; leaf 2 at hinge (4,0,40): `glRotatef(180 - gateOpen*70, 0,1,0)`; both call `LG_LEAF`. Target open = `(nearGate != gateToggle)` where `nearGate = distance to (0,40) < 7`. Eased at 0.7/s. Collision box x -4..4, y 0..2.6, z 39.85..40.15 while `gateOpen < 0.5`. E within 6.5 of the gate flips `gateToggle`.

### 9.2 Chandelier
Pivot `(2.0, WH, -17.6)`, sway `3*sin(1.1t)` degrees about Z. Chain `cyl(0.03,0.03,1.7)`, torus ring `glutSolidTorus(0.05,0.9,8,22)` laid flat (rotate 90 about X), hub sphere r=0.16, six arms (cylinders along +X, rotated i*60 about Y), a candle `cyl(0.04,0.04,0.24)` on each arm end with a flame.

### 9.3 Flames, candelabras
`flame(x,y,z,scale,phase)`: lighting off, orange `(1.0, 0.62+0.2f, 0.15)`, cone `0.028*s` radius, `0.11*s*f` height, where `f = 0.75 + 0.25 sin(13t+ph) + 0.15 sin(29t+2ph)`. Candelabras at (-2.05, F0+1.5, -15), (5, F0+0.8, -15.2), (5, F0+0.8, -27.5): base cylinders and crossbar are static (`LG_INT`), three candles each drawn dynamically with flames. Bedroom B candle flame at `(8.2, F1+0.78, -29.3)` scale 1.4.

### 9.4 Drapes (`drape(x,y,z,w,h,rot,phase,r,g,b,tear)`)
Quad strip, 12 sections. Top edge fixed at y; each bottom vertex offset by `sway = 0.10*sin(0.8t + u*5 + ph)` and fold `0.06*sin(20u+ph)`; bottom length `h*(1 - tear*0.3*sin²(9u+ph))` for torn edges. Instances (all hang from y = WH-0.1):
- Room B, either side of the bed: `(7.7,·,-29.6)` w1.0 h3.4 and `(11.3,·,-29.6)` w1.0 h3.2, colour `(0.10,0.12,0.17)`, tear 1.
- Room A curtain: `(-11.6,·,-14.6)` w1.0 h3.0 `(0.12,0.06,0.06)`.
- Room C canopy over the first iron bed: `(11.5,·,-23.3)` w1.2 h2.6 rot -90 `(0.28,0.24,0.18)`.

### 9.5 Rocking chair
At `(-8.7, F1, -19)` rotated 90 about Y; whole chair rotated `5*sin(1.3t)` degrees about X. Seat, back, four legs, two curved runners (7 boxes each following a parabola).

### 9.6 Lamp bulbs
Left lamp bulb at (-7,3.5,38): colour scaled by `fLamp`; right at (7,3.5,38) steady; room A hanging lamp bulb at (-9, WH-1.65, -22).

### 9.7 Train
Position: `zHead = -160 + 320*fmod(t,30)/30` (loops every 30 s, about 10.7 units/s, moving toward +Z). Car centres: loco `zHead-4`, carriage 1 `zHead-13`, carriage 2 `zHead-22`. For each car: translate to `(trackX(zc),0,zc)`, rotate by `trackYaw(zc)`.
- Locomotive (rust `(0.44,0.21,0.09)`): chassis box 2.4x0.35x8 at y=1.0; body `box(0,2.1,0.5, 2.6,2.4,5.0)`; nose `box(0,1.6,3.6, 2.2,1.6,1.6)`; cab `box(0,3.0,-2.5, 2.8,3.2,2.6)` with prism roof; chimney box; orange-yellow patch quads `(0.85,0.55,0.15)` on both sides; dark cab windows; emissive headlight sphere r=0.28 at `(0,2.0,4.45)`; wheels at z = -2.7,-0.9,1.3,3.0 on both sides (x=+-1.1, y=0.75); side rods.
- Carriages: `box(0,2.4,0, 2.8,2.8,8.2)`, roof slab, four dark window quads per side, 4 wheels per side.
- Wheel: rotate 90 about Y, spin `wheelAng = t*1273` degrees (about 10.7 u/s on radius 0.45), `gluCylinder` r=0.45 length 0.16 with two `gluDisk` caps, crossed spoke boxes.

### 9.8 Ghost
Position: `x = -30 + 6 sin(0.25t)`, `z = 23 + 6 cos(0.2t)`, `y = 1.5 + 0.3 sin t`. Alpha `0.35 + 0.15 sin(0.7t)`. Drawn last in the blended pass with lighting off, colour `(0.75,0.85,0.95,alpha)`: cone body (r 0.75, h 1.9), head sphere r=0.36, two arm cones, dark eyes and mouth. Rotated to face the player: `atan2(camX - x, camZ - z)`.

### 9.9 Bats and crows (same `winged()` routine)
- 5 bats orbit the tower top: centre (-13.5, 14.5, -11.8), radius `4.5 + 0.8i`, angle `t*(1.3+0.15i) + 1.3i`, bob `1.2 sin(2t+i)`. Wing flap `35 sin(12t+i)` degrees. Body scale 1.
- 6 crows circle the yard: centre (8, 12.5, 12), radius `15 + 3i`, angle `t*(0.35+0.03i) + 1.05i`, flap `22 sin(4.5t+i)`. Scale 2.4, colour near black.
- Heading: rotate `-(theta in degrees) - 90` about Y (body faces local +X, wings along +-Z).
- `winged`: ellipsoid body + head sphere + two wing triangles rotated about X by `+-flap`.

### 9.10 Moonlight beams (transparent, depth write off)
Six translucent quads that slope from a window down to the upstairs floor:

| Window (wx, wy, wz) | dir |
|---|---|
| (-12, 6.6, -18) / (-12, 6.6, -22) / (-12, 6.6, -26) | +1 (into room A) |
| (12, 6.6, -27.5) | -1 (room B) |
| (12, 6.6, -18) / (12, 6.6, -22) | -1 (room C) |

Quad: top edge at `x = wx + dir*0.32, y = wy+0.8, z = wz +- 0.55`, alpha 0.16; bottom edge at `x = wx + dir*4.2, y = F1+0.03, z = wz +- 0.55 + 0.9`, alpha 0.05; colour `(0.55,0.68,0.95)`, slow pulse `0.8 + 0.2 sin(0.5t+i)`.

### 9.11 Low fog sheets
Seven large blended quads (32 x 32, alpha 0.055, colour `(0.30,0.42,0.42)`) at y = 0.45 + 0.15i, drifting with sines of time. Drawn with fog and lighting off.

---

## 10. Sky (drawn first each frame, camera-relative)

State: lighting off, fog off, depth test off, depth write off; translate to camera position.
1. Four side quads (rotated by 90 degrees each) of half-size 190, vertical gradient: horizon at y=-40 colour = fog base (+ lightning), mid at y=60 `(0.03,0.05,0.09)`, top at y=190 `(0.01,0.015,0.04)`; plus a top cap quad at y=190.
2. 200 stars (points, size 2), unit dome radius 150 with elevation in [0.12,1.4] radians, twinkle `0.55 + 0.45 sin(1.5t + i)`.
3. Moon: blended halo spheres radius 22 (alpha 0.05, colour `(0.5,0.6,0.8)`) and 15 (alpha 0.08), then near-white sphere radius 9 `(0.96,0.96,0.90)` plus two grey craters. Position `normalize(-0.30,0.36,-0.88) * 150`. Use 40 slices.
4. 9 drifting cloud quads at y = 70..86, size 180 x 60, alpha 0.30, scrolling in X.

Lightning adds `flashV` times a boost to every sky colour. Restore depth test, fog and lighting after.

---

## 11. Lighting (all 8 fixed-function lights)

Global setup in `init`: `GL_LIGHTING`, `GL_NORMALIZE`, `GL_COLOR_MATERIAL` with `glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE)`, `GL_LIGHT_MODEL_TWO_SIDE = TRUE`, smooth shading.

Global ambient each frame: `(0.16 - 0.04*indoor + 0.5*flash, 0.19 - 0.06*indoor + 0.5*flash, 0.27 - 0.10*indoor + 0.55*flash)`.

**Call order each frame in `display()` (critical):**
1. `glLoadIdentity()`.
2. Set the flashlight (LIGHT1) **now**, in eye space: position `(0,0,0,1)`, direction `(0,0,-1)`, cutoff 22, exponent 12, diffuse `(1.2,1.2,1.05)`, attenuation const 0.5, linear 0.04, quad 0.004.
3. `gluLookAt(...)`.
4. Set all other lights (world space) and enable/disable by distance.

| Light | Type and position | Colour (diffuse) | Attenuation (c, l, q) | Enabled when player within |
|---|---|---|---|---|
| 0 Moon | Directional `(-0.4,1.0,-0.3,0)` | `(0.30,0.38,0.55) * (1 - 0.85*indoor + 1.5*flash)` | none | always |
| 1 Flashlight | Spot, eye space (above) | pale white | 0.5, 0.04, 0.004 | when F is on |
| 2 Chandelier | Point `(2 + 0.1 sin(1.1t), WH-2.4, -17.6)` | `(1.5,1.0,0.45) * fChand` | 0.5, 0.04, 0.018 | 20 units |
| 3 Bedroom candle | Point `(8.2, F1+1.0, -29.3)` | `(1.4,0.65,0.18) * fCandle` | 0.4, 0.10, 0.08 | 16 units |
| 4 Lamp post | Point `(-7, 3.4, 38)` | `(1.3,1.0,0.45) * fLamp` | 0.6, 0.04, 0.012 | 45 units |
| 5 Train headlight | Spot at head nose `(trackX(zc)+sin(yaw)*4.5, 2.0, zc+cos(yaw)*4.5)`, direction `(sin(yaw), -0.04, cos(yaw))`, cutoff 30, exponent 6 | `(1.5,1.4,1.0)` | 1.0, 0.02, 0.0015 | 110 units |
| 6 Window glow | Point `(0, 4.5, -10.5)` | `(0.55,0.38,0.14)` | 0.6, 0.05, 0.03 | 30 units |
| 7 Ghost glow | Point at ghost position | `(0.35,0.5,0.7)` | 0.6, 0.06, 0.04 | 26 units |

Flicker values (per frame, in `update`):
```
fChand  = 0.85 + 0.10 sin(9t) + 0.06 sin(23t+1) + 0.05*(rnd-0.5)
fCandle = 0.65 + 0.20 sin(13t) + 0.12 sin(31t+2) + 0.20*(rnd-0.5)
fLamp   = (fmod(t,7) < 0.3) ? 0.25 + 0.6*|sin(45t)| : 1.0     // occasional flicker
```
`indoorAmt` eases toward 1 when the player is inside the mansion footprint (`|px|<11.9 && -29.9<pz<-14.1`) at rate 3/s, else toward 0.

---

## 12. Camera, movement, collision, floor height

### 12.1 `update(dt)` order
1. `simTime += dt`; compute `fwd` vector; apply arrow-key look.
2. Update doors (ease `open`), door 1 auto-open, gate.
3. Walking: direction from W/A/S/D (normalised), `speed = (shift?6:3)*dt`, head bob phase `+= (shift?11:7)*dt`.
4. Build `extra` colliders (closed doors, closed gate, five fence boxes), run `collide`.
5. Clamp to +-120, then floor height easing: `feet += (target-feet)*min(1, dt*(target>feet ? 14 : 8))`.
6. Fog density and indoor amount easing (fog rate 2/s).
7. Flicker values, lightning.

Clamp `dt` to 0.1 in `idle`.

### 12.2 Collision
Player is a circle of radius 0.35 in XZ. A box is tested only if the player's vertical span overlaps it: `feet+1.6 > box.y0 && feet+0.3 < box.y1`. Resolve by clamping the player centre to the box, and pushing out along the vector to the closest point (if the centre is inside the box, push out the shortest axis). Run 3 iterations over all boxes (`solids` + `extra`).

Fence boxes (y 0..2.6, thickness 0.2): front left x -40.2..-4, front right x 4..40.2 (both z 39.9..40.1), back z -40.1..-39.9, east x 39.9..40.1, west x -40.1..-39.9.

Register a collider with every `wallBox` (all mansion walls), plus the items named in sections 6 to 8 (trees, tombstones, wagon, tower, turret, porch columns, stair rail, gallery rail, tables, beds, dresser, wardrobe).

### 12.3 `floorH(x, z, feet)`
```
if |x|<2.2 && -13.8 < z <= -11.5:  return 0.06 * clamp(ceil((-11.5 - z)/0.5), 0, 5)   // porch steps
inHouse = |x|<12.2 && -30.2 < z < -13.8
if !inHouse: return 0
if -6 < x < -2 && -21.3 < z <= -15:                                                    // stairs
     idx = clamp(ceil((-15 - z)/0.45), 0, 14);  return F0 + 0.3*idx
upper = (|x|<6.2 && z<-21.3) || |x|>5.8         // gallery or a bedroom wing
if upper && feet > 2.6: return F1
return F0
```
The `feet > 2.6` test keeps the player on the ground floor when walking underneath the gallery.

### 12.4 Viewpoints (`setView`)

| Key | Position (x, z), feet | Yaw / pitch | Shows |
|---|---|---|---|
| 1 Gate | (0, 46), 0 | 0 / 0.03 | Gate and path (R2/R3) |
| 2 Hall | (0.5, -15.6), F0 | 0 / 0.06 | Stairs, gallery (R6) |
| 3 Bedroom | (7.2, -26.0), F1 | 30 degrees / -0.05 | Room B bed and candle (R9-R11) |
| 4 Railway | (trackX(50)-2.4, 50), 0 | 0.14 / 0.03 | Curved track, train coming toward you (R12/R13) |

Start: view 1 (player stands at (0,46), just outside the gate).

---

## 13. Fog and atmosphere

- `glEnable(GL_FOG)`, `GL_EXP2`, colour `(0.08,0.12,0.13)` (+ lightning), `GL_FOG_HINT = GL_NICEST`. Clear colour equals fog colour.
- Density target: inside mansion **0.006**, near railway (`px > 42`) **0.03**, otherwise **0.018**. Ease toward target with `fogDens += (target - fogDens)*min(1, dt*2)`.
- Lightning: `nextFlash -= dt`; when it hits 0 start `flashT = 0` and set `nextFlash = random 12..28 s`. While `flashT >= 0`: `flashV = 1` for `flashT < 0.06` or `0.09 < flashT < 0.15`, else 0; end at 0.2. L key sets `flashT = 0`.
- "Shadows": only dark blob quads under trees (no real shadows in OpenGL 1.x).

---

## 14. Rendering order in `display()`

1. Compute fog/clear colour with lightning; `glClear`.
2. `glLoadIdentity`; flashlight; `gluLookAt`; ambient; fog colour/density; world lights.
3. `drawSky`.
4. Enable lighting, fog, depth test, depth write.
5. Opaque static lists: `GROUND, FENCE, PATH, TREES, GRAVE, PROPS, EXT, INT, RAIL, PINES`.
6. Opaque dynamic: gate, four doors, chandelier, three candelabras, drapes, rocking chair, bedroom B candle, lamp bulbs, train.
7. Blend on (`SRC_ALPHA, ONE_MINUS_SRC_ALPHA`), depth write **off**: beams, fog sheets, ghost. Then depth write on, blend off.
8. Bats and crows (opaque).
9. HUD: ortho 2D, lighting/fog/depth off. Top line: title, zone name, flashlight state, fps. Bottom two lines: controls (toggle with H).
10. `glutSwapBuffers`.

Zone name for HUD: "Mansion hall" / "Upstairs" (inside footprint, by `feet>3`), "Railway / forest" (`px>42`), "Outside the fence" (`|px|>40 || |pz|>40`), else "Haunted grounds".

---

## 15. Colour palette (reference)

| Element | RGB (0..1) |
|---|---|
| Night sky / clear colour | 0.03, 0.05, 0.08 (horizon = fog) |
| Fog | 0.08, 0.12, 0.13 |
| Moonlight | 0.30, 0.38, 0.55 |
| Window / candle glow | 1.00, 0.78, 0.30 |
| Stone | 0.32, 0.32, 0.34 |
| Dark wood | 0.25, 0.16, 0.09 |
| Rust | 0.45, 0.22, 0.10 |
| Dead grass | 0.06, 0.14, 0.07 |
| Ghost | 0.75, 0.85, 0.95, alpha 0.35 to 0.5 |

---

## 16. Build order for an implementer

Do these in order and run after each step.

| Step | Work | Check |
|---|---|---|
| 1 | Window, projection, camera, mouse/keys, ground, sky, moon | Look around a dark field |
| 2 | Primitive helpers, `bigBox`, materials | Test cubes light correctly |
| 3 | Fence, pillars, gargoyles, gate, path | Estate is enclosed |
| 4 | Mansion walls, roof, gables, dormers, chimneys, tower, turret, porch, windows | House looks right from the gate |
| 5 | Dead trees, graveyard, props, lamp posts | Yard looks abandoned |
| 6 | Hall floor, walls, staircase, gallery, rails, portraits, chandelier | Stairs and gallery look right |
| 7 | Three bedrooms and furniture | Each room matches its description |
| 8 | Railway, train, pines | Train loops on the curve |
| 9 | All eight lights, flicker, per-distance enabling | Each light identifiable |
| 10 | Doors, gate, ghost, bats, crows, drapes, chair, beams, fog sheets | Everything animates |
| 11 | Collision, `floorH`, fog zones, lightning, HUD, viewpoints | Cannot leave estate or pass walls; stairs walkable |
| 12 | `--shot` test mode, tuning, performance pass | Screenshots match section 17 |

---

## 17. Acceptance checklist and known pitfalls

### Checklist
- [ ] Compiles with no errors (`-Wall` gives only harmless warnings).
- [ ] Start view shows the gate, gargoyle pillars, path, moon, and the faint mansion through fog.
- [ ] View 2 shows staircase on the left, gallery above, side table with lit candles.
- [ ] View 3 shows a bed, candle, drapes, a moonlit window, and dithering-free walls.
- [ ] View 4 shows the curved track fading into fog with the train headlight in the distance.
- [ ] Player cannot leave the fence unless the gate is open; cannot walk through walls, doors (when closed), furniture or trees.
- [ ] Player can climb the stairs smoothly and walk into all three bedrooms through the gallery doors.
- [ ] E opens/closes doors and gate; door 1 creaks open when approached upstairs.
- [ ] F toggles the flashlight; fog density visibly changes between yard, inside, railway.
- [ ] Train loops every 30 s; lightning flashes at random and with L.
- [ ] About 60 fps on a normal PC.

### Pitfalls (each of these was hit during development)
1. **Per-vertex lighting:** without subdivision (`bigBox`, grid ground), spot and point lights vanish on large faces.
2. **Z-fighting:** keep decorations 0.02 to 0.1 units off surfaces; wallpaper linings must be in the room, not inside the wall (see 8.6); windows sit 0.3 inside, so linings never hide them.
3. **Grass tufts inside the hall:** exclude the house footprint (6.1).
4. **Coplanar wall ends:** stop front and back walls at x=+-11.8 so side walls own the corners.
5. **Light call order:** flashlight before `gluLookAt`, everything else after.
6. **Sky needs fog off and depth off**, or it fades to a flat colour and hides the scene.
7. **Railway viewpoint:** stand 2.4 units beside the track and look along it; standing 9 units away puts pine trunks in the way.
8. **Yaw/rotation signs:** re-read section 3.3 before placing rotated objects (beds, doors, windows).
9. **Windows/Linux headers:** avoid the names `near`/`far`; on macOS include `<GLUT/glut.h>`.
10. **Mouse look:** ignore the synthetic motion event created by `glutWarpPointer`, or the camera spins.

### Known limitations and future work
Primitive shapes only approximate the reference photos; GLUT has no audio; no real shadows or volumetric fog without shaders; ground-floor wings of the mansion are sealed. Possible extensions: footstep/door/train sounds, a "find the rusty key in Bedroom B to open the gate" objective in the style of *Granny*, procedural textures for stone, planks and cobblestones.
